#include "test_fast.h"

#include "dataset_loader.h"
#include "feature_extraction.h"
#include "file_system.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>

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
    namespace fast {
        void test_with_dataset(uint8_t* image_data_buffer,
                               image::KeyPoint* keypoints_buffer,
                               const size_t keypoints_buffer_size,
                               const char* dataset_name) {

            if (!file_system::cd("/")) {
                logger::errorf("Failed to set current directory\r\n");
                return;
            }

            char features_file_path[16] = "";
            sprintf(features_file_path, "%s.dat", dataset_name);

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
            size_t total_features_extracted = 0;
            size_t total_reference_features = 0;

            uint64_t total_cycles = 0;

            while (line_ptr != NULL) {
                image::Image image;

                // The reason this is not profiled is due to in a real pipeline
                // where the image is retrieved from the camera, the camera
                // driver would write directly to the staticly allocated
                // image_data buffer. In order to not have to modify LodePNG too
                // much (it uses the heap), this is kept in this way where the
                // data is copied over and then freed
                {
                    image::Image image_heap;

                    if (!dataset_loader::retrieve_image(image_heap)) {
                        logger::errorf("Failed to rerieve image\r\n");

                        *line_ptr++ = '\0';
                        data_ptr    = line_ptr;
                        line_ptr    = strchr(data_ptr, '\n');

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

                uint32_t keypoints_size = keypoints_buffer_size;

                profile_start();
                frontend::extract_features(image.data,
                                           image.width,
                                           image.height,
                                           100,
                                           keypoints_buffer,
                                           &keypoints_size);
                uint32_t cycles = profile_end();

                total_features_extracted += keypoints_size;

                total_cycles += cycles;

                // Now compare the output with the line from the features file

                // Substitute the line feed with a null termination so we can
                // run strtok on the current line
                *line_ptr++ = '\0';

                char* token = strtok(data_ptr, ",");

                uint16_t reference_features[keypoints_buffer_size * 2];
                size_t reference_features_size = 0;

                // Extract the reference features
                while (token != NULL) {

                    if (reference_features_size ==
                        sizeof(reference_features) /
                            sizeof(reference_features[0])) {
                        logger::errorf("Reference feature won't fit in "
                                       "reference buffer!\r\n");
                        exit(1);
                    }

                    reference_features[reference_features_size++] =
                        strtol(token, NULL, 10);

                    token = strtok(NULL, ",");
                }

                size_t features_matching = 0;

                for (size_t i = 0; i < keypoints_size; i++) {

                    image::KeyPoint& keypoint = keypoints_buffer[i];

                    const uint16_t x = (uint16_t)round(keypoint.point.x);
                    const uint16_t y = (uint16_t)round(keypoint.point.y);

                    for (size_t j = 0; j < reference_features_size; j += 2) {

                        if (x == reference_features[j] &&
                            y == reference_features[j + 1]) {

                            features_matching++;
                            break;
                        }
                    }
                }

                double ms = 1000.0 *
                            (double)((double)cycles /
                                     (double)BOARD_BOOTCLOCKRUN_CORE_CLOCK);

                logger::infof("%d: Extracting %d features took %f. Features "
                              "matching: %d/%d\r\n",
                              iterations,
                              keypoints_size,
                              ms,
                              features_matching,
                              reference_features_size / 2);

                total_features_matched += features_matching;
                total_reference_features += reference_features_size / 2;

                logger::infof("%d: Total match rate: %f\r\n",
                              iterations,
                              (double)total_features_matched /
                                  (double)total_reference_features);

                data_ptr = line_ptr;
                line_ptr = strchr(data_ptr, '\n');

                iterations++;
            }

            double ms = 1000.0 *
                        (double)((double)total_cycles /
                                 ((double)BOARD_BOOTCLOCKRUN_CORE_CLOCK *
                                  (double)iterations));

            logger::rawf("\r\n");
            logger::infof(
                "Features matched: %d/%d across %d images. Total number of "
                "features extracted: %d. Average features extracted per "
                "image: %f. Average runtime: %f \r\n",
                total_features_matched,
                total_reference_features,
                iterations,
                total_features_extracted,
                (float)total_features_extracted / (float)iterations,
                ms);

            free(features_file_content_buffer);

            dataset_loader::deinitialise();
        }

        void
        test_with_dataset_without_references(uint8_t* image_data_buffer,
                                             image::KeyPoint* keypoints_buffer,
                                             const size_t keypoints_buffer_size,
                                             const char* dataset_name) {

            char dataset_path[16] = "";
            sprintf(dataset_path, "/%s", dataset_name);

            dataset_loader::initialise(dataset_path);

            logger::infof("Keypoints exracted: \r\n");

            while (true) {
                image::Image image;

                {
                    image::Image image_heap;

                    if (!dataset_loader::retrieve_image(image_heap)) {
                        logger::errorf("Failed to rerieve image\r\n");
                        break;
                    }

                    image.data   = image_data_buffer;
                    image.width  = image_heap.width;
                    image.height = image_heap.height;

                    memcpy(image.data,
                           image_heap.data,
                           image.width * image.height);

                    free(image_heap.data);
                }

                uint32_t keypoints_size = keypoints_buffer_size;

                frontend::extract_features(image.data,
                                           image.width,
                                           image.height,
                                           100,
                                           keypoints_buffer,
                                           &keypoints_size);

                for (size_t i = 0; i < keypoints_size; i++) {

                    if (i == keypoints_size - 1) {
                        logger::rawf("%d, %d",
                                     (int)round(keypoints_buffer[i].point.x),
                                     (int)round(keypoints_buffer[i].point.y));
                    } else {
                        logger::rawf("%d, %d, ",
                                     (int)round(keypoints_buffer[i].point.x),
                                     (int)round(keypoints_buffer[i].point.y));
                    }
                }

                logger::rawf("\r\n");
            }

            dataset_loader::deinitialise();
        }
    }
}
