#include "test_lucas_kanade.h"

#include "dataset_loader.h"
#include "feature_extraction.h"
#include "feature_tracking.h"
#include "file_system.h"
#include "logger.h"

#include <stdlib.h>

#define MAX_AMOUNT_OF_REFERENCE_POINTS (800)

static void populate_buffer_from_data_entry(char* data,
                                            int* buffer,
                                            const size_t buffer_size,
                                            size_t* entries) {

    char* token = strtok(data, ",");

    size_t entry_index = 0;

    // Extract the reference tracked features
    while (token != NULL) {

        if (entry_index == buffer_size) {
            logger::errorf(
                "Buffer doesn't have enough space for the data entry\r\n");
            exit(1);
        }

        buffer[entry_index++] = strtol(token, NULL, 10);

        token = strtok(NULL, ",");
    }

    *entries = entry_index;
}

static void profile_start() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0UL;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t profile_end() {
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    return DWT->CYCCNT;
}

namespace test {
    namespace lucas_kanade {
        void test_with_dataset(uint8_t* image_data_buffer,
                               uint8_t* image_pyramid_buffer,
                               image::PatchPyramid* patch_pyramid,
                               image::KeyPoint* keypoints_buffer,
                               image::KeyPoint* end_keypoints_buffer,
                               const size_t keypoints_buffer_size,
                               const char* dataset_name) {

            if (!file_system::cd("/")) {
                logger::errorf("Failed to set current directory\r\n");
                return;
            }

            char features_file_path[16] = "";
            sprintf(features_file_path, "%s_tracked.dat", dataset_name);

            uint32_t features_file_size = 0;
            if (!file_system::size(features_file_path, &features_file_size)) {
                logger::errorf("Failed to read features file size\r\n");
                return;
            }

            const uint32_t features_file_content_size =
                ((features_file_size / 512) + 1) * 512;

            uint8_t* features_file_content_buffer = (uint8_t*)malloc(
                features_file_content_size);

            if (!file_system::read(features_file_path,
                                   features_file_content_buffer,
                                   features_file_size)) {

                free(features_file_content_buffer);
                return;
            }

            // Append null termination for string operations
            features_file_content_buffer[features_file_size] = 0;

            char dataset_path[16] = "";
            sprintf(dataset_path, "/%s", dataset_name);

            dataset_loader::initialise(dataset_path);

            // For every image, we want to read until the end of the line
            char* data_ptr = (char*)features_file_content_buffer;
            char* line_ptr = strchr(data_ptr, '\n');

            size_t iterations = 0;

            size_t total_features_matched   = 0;
            size_t total_features_tracked   = 0;
            size_t total_reference_features = 0;

            uint64_t total_cycles = 0;

            size_t index = 1;

            double error = 0;

            image::Image previous_image, current_image;

            if (!dataset_loader::retrieve_image(previous_image, index)) {
                logger::errorf("Failed to first image\r\n");
                return;
            }

            while (line_ptr != NULL) {

                logger::rawf("\r\n");

                uint32_t cycles = 0;

                // First retrieve the start points

                // Substitute the line feed with a null termination so we can
                // run strtok on the current line
                *line_ptr++ = '\0';

                int reference_start_points[MAX_AMOUNT_OF_REFERENCE_POINTS * 2];
                int reference_end_points[MAX_AMOUNT_OF_REFERENCE_POINTS * 2];

                size_t reference_points_size = 0;

                // Grab the start points
                populate_buffer_from_data_entry(
                    data_ptr,
                    reference_start_points,
                    sizeof(reference_start_points) /
                        sizeof(reference_start_points[0]),
                    &reference_points_size);

                // Move the data pointer to the next line
                data_ptr    = line_ptr;
                line_ptr    = strchr(data_ptr, '\n');
                *line_ptr++ = '\0';

                // Then the end points
                populate_buffer_from_data_entry(
                    data_ptr,
                    reference_end_points,
                    sizeof(reference_end_points) /
                        sizeof(reference_end_points[0]),
                    &reference_points_size);

                reference_points_size /= 2;

                uint32_t keypoints_size = reference_points_size;

                for (size_t i = 0; i < keypoints_size; i++) {
                    keypoints_buffer[i].point.x = reference_start_points[i * 2];
                    keypoints_buffer[i].point.y =
                        reference_start_points[i * 2 + 1];
                    keypoints_buffer[i].stale = false;
                }

                {
                    // First build the patch pyramid from features in the first
                    // image
                    image::Image image;

                    image.data   = image_data_buffer;
                    image.width  = previous_image.width;
                    image.height = previous_image.height;

                    memcpy(image.data,
                           previous_image.data,
                           image.width * image.height);

                    // Construct image pyramid from the image. This is not
                    // counted in the profiled time since in a regular pipeline,
                    // the extraction would happen last after the image pyramid
                    // from the current image has been created
                    image::ImagePyramid image_pyramid(image,
                                                      image_pyramid_buffer);

                    if (keypoints_size > keypoints_buffer_size) {

                        logger::warnf(
                            "Keypoints in image does not fit within the "
                            "maximum amount of pyramid levels: %lu/%u. "
                            "Reducing size to max.\r\n",
                            keypoints_size,
                            keypoints_buffer_size);

                        keypoints_size = keypoints_buffer_size;
                    }

                    profile_start();
                    patch_pyramid->construct(image_pyramid,
                                             keypoints_buffer,
                                             keypoints_size);
                    cycles += profile_end();
                }

                // Now we can load the second image

                // The reason this is not profiled is due to in a real pipeline
                // where the image is retrieved from the camera, the camera
                // driver would write directly to the staticly allocated
                // image_data buffer and notify when the operation is done. This
                // would happen asynchronously with DMA and interrupts. In order
                // to not have to modify LodePNG too much (it uses the heap),
                // this is kept in this way where the data is copied over and
                // then freed
                image::Image image;

                {
                    if (!dataset_loader::retrieve_image(current_image,
                                                        index + 1)) {
                        logger::errorf("Failed to retrieve image\r\n");

                        index += 2;

                        // Skip two indices. Two rows per index, so have to skip
                        // 3 to point at the fourth
                        for (uint8_t k = 0; k < 2; k++) {
                            data_ptr    = line_ptr;
                            line_ptr    = strchr(data_ptr, '\n');
                            *line_ptr++ = '\0';
                        }

                        data_ptr = line_ptr;
                        line_ptr = strchr(data_ptr, '\n');

                        free(previous_image.data);

                        while (!dataset_loader::retrieve_image(previous_image,
                                                               index)) {

                            logger::errorf("Failed to retrieve image\r\n");

                            *line_ptr++ = '\0';
                            data_ptr    = line_ptr;
                            line_ptr    = strchr(data_ptr, '\n');
                            *line_ptr++ = '\0';
                            data_ptr    = line_ptr;
                            line_ptr    = strchr(data_ptr, '\n');
                            index += 1;
                        }

                        continue;
                    }

                    image.data   = image_data_buffer;
                    image.width  = current_image.width;
                    image.height = current_image.height;

                    memcpy(image.data,
                           current_image.data,
                           image.width * image.height);
                }

                index++;

                // Now we can do the actual tracking

                // The image pyramid creation is in this instance is profiled
                // since we're working on the "current" image
                profile_start();
                image::ImagePyramid image_pyramid(image, image_pyramid_buffer);

                frontend::track_features(*patch_pyramid,
                                         image_pyramid,
                                         keypoints_buffer,
                                         end_keypoints_buffer,
                                         keypoints_size);

                cycles += profile_end();

                if (index % 10 == 0) {

                    for (size_t i = 0; i < keypoints_size; i++) {

                        int* reference_start_point =
                            &reference_start_points[i * 2];
                        int* reference_end_point = &reference_end_points[i * 2];
                        image::KeyPoint end_point = end_keypoints_buffer[i];

                        if (reference_end_point[0] !=
                                round(end_point.point.x) ||
                            reference_end_point[1] !=
                                round(end_point.point.y)) {
                            printf("%2u: [%3.0f, %3.0f] -> [%3.0f, %3.0f] != "
                                   "[%3.0f, "
                                   "%3.0f] "
                                   "-> [%3.0f, %3.0f]\r\n",
                                   i,
                                   round(reference_start_point[0]),
                                   round(reference_start_point[1]),
                                   round(end_point.point.x),
                                   round(end_point.point.y),
                                   round(reference_start_point[0]),
                                   round(reference_start_point[1]),
                                   round(reference_end_point[0]),
                                   round(reference_end_point[1]));

                        } else {
                            printf("%2u: [%3.0f, %3.0f] -> [%3.0f, %3.0f] == "
                                   "[%3.0f, "
                                   "%3.0f] "
                                   "-> [%3.0f, %3.0f]\r\n",
                                   i,
                                   round(reference_start_point[0]),
                                   round(reference_start_point[1]),
                                   round(end_point.point.x),
                                   round(end_point.point.y),
                                   round(reference_start_point[0]),
                                   round(reference_start_point[1]),
                                   round(reference_end_point[0]),
                                   round(reference_end_point[1]));
                        }
                    }
                }

                total_features_tracked += keypoints_size;

                size_t tracked_features_matching = 0;

                // Now we can compare the result

                for (size_t i = 0; i < keypoints_size; i++) {

                    image::KeyPoint& keypoint = end_keypoints_buffer[i];

                    const int x = (int)round(keypoint.point.x);
                    const int y = (int)round(keypoint.point.y);

                    const int* reference_end_point =
                        &reference_end_points[i * 2];

                    if (x == reference_end_point[0] &&
                        y == reference_end_point[1]) {
                        tracked_features_matching++;
                    } else {
                        double dx = (double)x - (double)reference_end_point[0];
                        double dy = (double)y - (double)reference_end_point[1];

                        error += (dx * dx + dy * dy);
                    }
                }

                double ms = 1000.0 *
                            (double)((double)cycles /
                                     (double)BOARD_BOOTCLOCKRUN_CORE_CLOCK);

                total_features_matched += tracked_features_matching;
                total_reference_features += reference_points_size;

                logger::infof("%d: Tracking %d features took %.3f. Features "
                              "matching: %d/%d\r\n",
                              iterations,
                              keypoints_size,
                              ms,
                              tracked_features_matching,
                              reference_points_size);

                logger::infof(
                    "%d: Total track rate: %.4f. RMSE over mismatched "
                    "features: %.4f. Total RMSE: %.4f\r\n",
                    iterations,
                    (double)total_features_matched /
                        (double)total_reference_features,
                    sqrt(error / (double)(total_features_tracked -
                                          total_features_matched)),
                    sqrt(error / (double)total_features_tracked));

                data_ptr = line_ptr;
                line_ptr = strchr(data_ptr, '\n');

                iterations++;

                total_cycles += cycles;

                free(previous_image.data);
                previous_image.data = current_image.data;

                double total_ms =
                    1000.0 * (double)((double)total_cycles /
                                      ((double)BOARD_BOOTCLOCKRUN_CORE_CLOCK));

                logger::infof("Total ms: %f, average ms: %f\r\n",
                              total_ms,
                              total_ms / iterations);
            }

            double total_ms = 1000.0 *
                              (double)((double)total_cycles /
                                       ((double)BOARD_BOOTCLOCKRUN_CORE_CLOCK));

            double ms = 1000.0 *
                        (double)((double)total_cycles /
                                 ((double)BOARD_BOOTCLOCKRUN_CORE_CLOCK *
                                  (double)iterations));

            logger::rawf("\r\n");
            logger::infof(
                "Features matched: %d/%d across %d images. Total number of "
                "features tracked: %d. Average features tracked per "
                "image: %f. Total runtime: %f, Average runtime: %f \r\n",
                total_features_matched,
                total_reference_features,
                iterations,
                total_features_tracked,
                (double)total_features_tracked / (double)iterations,
                total_ms,
                ms);

            logger::infof("Total track rate: %.4f. RMSE over mismatched "
                          "features: %.4f. Total RMSE: %.4f\r\n",
                          (double)total_features_matched /
                              (double)total_reference_features,
                          sqrt(error / (double)(total_reference_features -
                                                total_features_matched)),
                          sqrt(error / (double)total_reference_features));

            free(features_file_content_buffer);

            dataset_loader::deinitialise();
        }

        void test_with_dataset_without_references_with_resample(
            uint8_t* image_data_buffer,
            uint8_t* image_pyramid_buffer,
            image::PatchPyramid* patch_pyramid,
            image::KeyPoint* keypoints_buffer,
            image::KeyPoint* end_keypoints_buffer,
            const size_t keypoints_buffer_size,
            const char* dataset_name,
            const size_t start_index,
            const size_t end_index) {

            char dataset_path[16] = "";
            sprintf(dataset_path, "/%s", dataset_name);

            dataset_loader::initialise(dataset_path);

            size_t index = start_index;

            uint32_t keypoints_size = 0;

            size_t stale_features = 0;

            while (index <= end_index) {

                /*
                if (index % 100 == 0) {
                    keypoints_size = 0;
                }
                */

                image::Image image;

                {
                    image::Image image_heap;

                    if (!dataset_loader::retrieve_image(image_heap, index)) {
                        logger::errorf("Failed to rerieve image\r\n");
                        index++;
                        continue;
                    }

                    image.data   = image_data_buffer;
                    image.width  = image_heap.width;
                    image.height = image_heap.height;

                    memcpy(image.data,
                           image_heap.data,
                           image.width * image.height);

                    free(image_heap.data);
                }

                image::ImagePyramid image_pyramid(image, image_pyramid_buffer);

                if (keypoints_size > 0) {
                    frontend::track_features(*patch_pyramid,
                                             image_pyramid,
                                             keypoints_buffer,
                                             end_keypoints_buffer,
                                             keypoints_size);

                    stale_features = 0;
                    for (size_t i = 0; i < keypoints_size; i++) {
                        if (end_keypoints_buffer[i].stale) {
                            stale_features++;
                        }
                    }

                    if (((int)keypoints_size - (int)stale_features) >= 5) {

                        logger::rawf("%lu: ", index);
                        for (size_t i = 0; i < keypoints_size; i++) {

                            if (i == keypoints_size - 1) {
                                logger::rawf(
                                    "%d, %d, %d",
                                    (int)round(end_keypoints_buffer[i].point.x),
                                    (int)round(end_keypoints_buffer[i].point.y),
                                    end_keypoints_buffer[i].stale ? 1 : 0);
                            } else {
                                logger::rawf(
                                    "%d, %d, %d, ",
                                    (int)round(end_keypoints_buffer[i].point.x),
                                    (int)round(end_keypoints_buffer[i].point.y),
                                    end_keypoints_buffer[i].stale ? 1 : 0);
                            }
                        }
                        logger::rawf("\r\n");
                    }

                    memcpy(keypoints_buffer,
                           end_keypoints_buffer,
                           sizeof(image::KeyPoint) * keypoints_size);
                }

                // if (keypoints_size == 0) {
                if (((int)keypoints_size - (int)stale_features) < 5) {
                    keypoints_size = keypoints_buffer_size;

                    frontend::extract_features(image.data,
                                               image.width,
                                               image.height,
                                               70,
                                               keypoints_buffer,
                                               &keypoints_size);

                    stale_features = 0;

                    logger::rawf("%lu: ", index);

                    for (size_t i = 0; i < keypoints_size; i++) {

                        if (i == keypoints_size - 1) {
                            logger::rawf(
                                "%d, %d, %d",
                                (int)round(keypoints_buffer[i].point.x),
                                (int)round(keypoints_buffer[i].point.y),
                                keypoints_buffer[i].stale ? 1 : 0);
                        } else {
                            logger::rawf(
                                "%d, %d, %d, ",
                                (int)round(keypoints_buffer[i].point.x),
                                (int)round(keypoints_buffer[i].point.y),
                                keypoints_buffer[i].stale ? 1 : 0);
                        }
                    }

                    logger::rawf("\r\n");
                }

                patch_pyramid->construct(image_pyramid,
                                         keypoints_buffer,
                                         keypoints_size);

                index++;
            }

            dataset_loader::deinitialise();
        }
    }
}
