#include "image.h"

#include <math.h>

#ifdef CPU_MIMXRT1166DVM6A
    #include "board.h"
    #include "delay.h"
    #include "fsl_device_registers.h"
    #include "profile.h"
#else
    #define SECTION_ITCM
#endif

namespace image {

    SECTION_ITCM int_fast32_t inline clamp(int_fast32_t value,
                                           int_fast32_t min,
                                           int_fast32_t max) {

        if (value < min) {
            return min;
        }

        if (value > max) {
            return max;
        }

        return value;
    }

    void
    blur_inplace(uint8_t* image_data, size_t image_width, size_t image_height) {

        // Gaussian blur kernel:
        //
        // [1/16 1/8 1/16]
        // [ 1/8 1/4 1/8 ]
        // [1/16 1/8 1/16]
        //

        // Seperable kernel of the Gaussian blur for horizontal and vertical
        // convolution to reduce the amount of multiplication operations that
        // has to be done:
        //
        // [ 1/4 ]
        // [ 1/2 ] * [ 1/4 1/2 1/4] = Gaussian kernel
        // [ 1/4 ]
        //
        // The kernel is interpreted as the amount of right shift that has to be
        // done.
        const uint8_t kernel[3] = {2, 1, 2};

        // In order to do the convolution in place, we have a buffer for the
        // current rows operated on
        uint8_t row_buffer[image_width * 3];

        // The row buffer ptr is shifted along as we go through the r6365ows,
        // pointing to three rows in the row buffer which holds the image values
        // before being modified. This enables the convolution to be made in
        // place
        //
        // We calculate the pointers for every row to not having to do modulo
        // operations in the for loops below
        uint8_t* row_buffer_ptr[image_height];

        for (size_t i = 0; i < image_height; i++) {
            row_buffer_ptr[i] = &row_buffer[(i % 3) * image_width];
        }

        memcpy(row_buffer_ptr[0], &image_data[0], image_width);
        memcpy(row_buffer_ptr[1], &image_data[image_width], image_width);

        for (int_fast32_t y = 1; y < (int_fast32_t)image_height - 1; y++) {

            const int_fast32_t y0 = y - 1;
            const int_fast32_t y1 = y;
            const int_fast32_t y2 = y + 1;

            // Shift out the last used row with the next one
            memcpy(row_buffer_ptr[y2],
                   &image_data[y2 * image_width],
                   image_width);

            for (int_fast32_t x = 1; x < (int_fast32_t)image_width - 1; x++) {

                const int_fast32_t x0 = (x - 1);
                const int_fast32_t x1 = x;
                const int_fast32_t x2 = (x + 1);

                // These are the results from seperable horizontal convolution
                const uint8_t r00 = row_buffer_ptr[y0][x0] >> kernel[0];
                const uint8_t r01 = row_buffer_ptr[y0][x1] >> kernel[1];
                const uint8_t r02 = row_buffer_ptr[y0][x2] >> kernel[2];

                const uint8_t r10 = row_buffer_ptr[y1][x0] >> kernel[0];
                const uint8_t r11 = row_buffer_ptr[y1][x1] >> kernel[1];
                const uint8_t r12 = row_buffer_ptr[y1][x2] >> kernel[2];

                const uint8_t r20 = row_buffer_ptr[y2][x0] >> kernel[0];
                const uint8_t r21 = row_buffer_ptr[y2][x1] >> kernel[1];
                const uint8_t r22 = row_buffer_ptr[y2][x2] >> kernel[2];

                // Whereas these are the result from the seperable vertical
                // convolution
                const uint8_t r0 = (r00 + r01 + r02) >> kernel[0];
                const uint8_t r1 = (r10 + r11 + r12) >> kernel[1];
                const uint8_t r2 = (r20 + r21 + r22) >> kernel[2];

                image_data[y * image_width + x] = r0 + r1 + r2;
            }
        }
    }

    void blur_inplace(Image& image) {
        blur_inplace(image.data, image.width, image.height);
    }

    void downsample_inplace(Image& source) {

        const size_t new_width  = source.width >> 1;
        const size_t new_height = source.height >> 1;

        uint8_t row_buffer[source.width * 2];

        // Downscale by taking an average over every 2x2 block, thus we
        // process 2 rows of the original image for each new row in the
        // downscaled image
        for (int_fast32_t j = 0; j < (int_fast32_t)new_height; j++) {

            memcpy(&row_buffer[0],
                   &source.data[source.width * j * 2],
                   source.width);
            memcpy(&row_buffer[source.width],
                   &source.data[source.width * j * 2 + source.width],
                   source.width);

            for (int_fast32_t i = 0; i < (int_fast32_t)new_width; i++) {

                int_fast32_t sum =
                    ((int_fast32_t)row_buffer[i * 2] +
                     (int_fast32_t)row_buffer[i * 2 + 1] +
                     (int_fast32_t)row_buffer[source.width + i * 2] +
                     (int_fast32_t)row_buffer[source.width + i * 2 + 1]);

                source.data[j * new_width + i] = sum / 4;
            }
        }

        source.width  = new_width;
        source.height = new_height;
    }

    void downsample(const Image& source, Image& destination) {

        destination.width  = source.width >> 1;
        destination.height = source.height >> 1;

        // Downscale by taking an average over every 2x2 block, thus we
        // process 2 rows of the original image for each new row in the
        // downscaled image
        for (int_fast32_t j = 0; j < (int_fast32_t)destination.height; j++) {
            for (int_fast32_t i = 0; i < (int_fast32_t)destination.width; i++) {

                int_fast32_t sum =
                    ((int_fast32_t)source.data[j * 2 * source.width + i * 2] +
                     (int_fast32_t)
                         source.data[j * 2 * source.width + i * 2 + 1] +
                     (int_fast32_t)source
                         .data[j * 2 * source.width + source.width + i * 2] +
                     (int_fast32_t)source.data[j * 2 * source.width +
                                               source.width + i * 2 + 1]);

                destination.data[j * destination.width + i] = sum / 4;
            }
        }
    }
    // ------------------------- Image Pyramid -------------------------------

    ImagePyramid::ImagePyramid(const Image& source, uint8_t* pyramid_buffer) {

        images[0].width  = source.width;
        images[0].height = source.height;
        images[0].data   = source.data;

        size_t offset = 0;

        for (size_t i = 1; i < PYRAMID_LEVELS; i++) {
            images[i].data = pyramid_buffer + offset;

            downsample(images[i - 1], images[i]);

            offset += images[i].width * images[i].height;
        }

        // Blur the images
        for (size_t i = 0; i < PYRAMID_LEVELS; i++) {

            image::blur_inplace(images[i]);
        }
    }

    image::Image* ImagePyramid::at(const size_t pyramid_level) {
        return &images[pyramid_level];
    }

    // ---------------------------- Patch ------------------------------------

    Patch::Patch(const float x, const float y, const image::Image& image)
        : origin{x, y} {

        int_fast32_t start_point_x = floor(x);
        int_fast32_t start_point_y = floor(y);

        // First want to copy over the data and append an extra
        // border to the left and top edge for interpolation
        uint8_t
            buffer[(PATCH_SIZE_WITH_BORDER + 2) * (PATCH_SIZE_WITH_BORDER + 2)];

        for (int_fast32_t j = 0; j < (int_fast32_t)PATCH_SIZE_WITH_BORDER + 2;
             j++) {

            // Make sure that the patch don't go outside bounds of the
            // image. If that is the case, just extrapolate with the
            // intensity values at the border.
            int_fast32_t ys = clamp(start_point_y + j - 2, 0, image.height - 1);

            for (int_fast32_t i = 0;
                 i < (int_fast32_t)PATCH_SIZE_WITH_BORDER + 2;
                 i++) {
                int_fast32_t xs =
                    clamp(start_point_x + i - 2, 0, image.width - 1);

                buffer[j * (PATCH_SIZE_WITH_BORDER + 2) + i] =
                    image.data[ys * image.width + xs];
            }
        }

        float alpha_x = abs(x - start_point_x);
        float alpha_y = abs(y - start_point_y);

        for (int_fast32_t j = 0; j < (int_fast32_t)PATCH_SIZE_WITH_BORDER;
             j++) {

            for (int_fast32_t i = 0; i < (int_fast32_t)PATCH_SIZE_WITH_BORDER;
                 i++) {

                // Store values for interpolating at sub-pixel level
                const float I =
                    buffer[(j + 1) * (PATCH_SIZE_WITH_BORDER + 2) + i + 1];
                const float I_x_shift =
                    buffer[(j + 1) * (PATCH_SIZE_WITH_BORDER + 2) + i + 2];
                const float I_y_shift =
                    buffer[(j + 2) * (PATCH_SIZE_WITH_BORDER + 2) + i + 1];
                const float I_xy_shift =
                    buffer[(j + 2) * (PATCH_SIZE_WITH_BORDER + 2) + i + 2];

                data[j * PATCH_SIZE_WITH_BORDER + i] =
                    (1 - alpha_x) * (1 - alpha_y) * I +
                    alpha_x * (1 - alpha_y) * I_x_shift +
                    (1 - alpha_x) * alpha_y * I_y_shift +
                    alpha_x * alpha_y * I_xy_shift;
            }
        }
    }

    float* Patch::row(int index) {
        return &data[(index + 1) * (PATCH_SIZE_WITH_BORDER) + 1];
    }

    void Patch::print() {

        for (size_t j = 0; j < PATCH_SIZE_WITH_BORDER; j++) {
            for (size_t i = 0; i < PATCH_SIZE_WITH_BORDER; i++) {

                printf("%3.f ", data[j * PATCH_SIZE_WITH_BORDER + i]);
            }

            printf("\r\n");
        }
    }

    void dx(const Patch& source, Patch& destination) {

        destination.origin.x = source.origin.x;
        destination.origin.y = source.origin.y;

        float kernel[3][3] = {{-1, 1}, {-2, 2}, {-1, 1}};

        for (int_fast32_t j = 1; j < PATCH_SIZE + 1; j++) {
            for (int_fast32_t i = 1; i < PATCH_SIZE + 1; i++) {

                // Here we utilize the border so that we can take the gradient
                // on the patch without having to omit pixels
                destination.data[j * PATCH_SIZE_WITH_BORDER + i] =
                    source.data[(j - 1) * PATCH_SIZE_WITH_BORDER + (i - 1)] *
                        kernel[0][0] +
                    source.data[(j - 1) * PATCH_SIZE_WITH_BORDER + (i + 1)] *
                        kernel[0][1] +

                    source.data[j * PATCH_SIZE_WITH_BORDER + (i - 1)] *
                        kernel[1][0] +
                    source.data[j * PATCH_SIZE_WITH_BORDER + (i + 1)] *
                        kernel[1][1] +

                    source.data[(j + 1) * PATCH_SIZE_WITH_BORDER + (i - 1)] *
                        kernel[2][0] +
                    source.data[(j + 1) * PATCH_SIZE_WITH_BORDER + (i + 1)] *
                        kernel[2][1];
            }
        }
    }

    void dy(const Patch& source, Patch& destination) {
        destination.origin.x = source.origin.x;
        destination.origin.y = source.origin.y;

        float kernel[2][3] = {{-1, -2, -1}, {1, 2, 1}};

        for (int_fast32_t j = 1; j < PATCH_SIZE + 1; j++) {
            for (int_fast32_t i = 1; i < PATCH_SIZE + 1; i++) {

                // Here we utilize the border so that we can take the gradient
                // on the patch without having to omit pixels
                destination.data[j * PATCH_SIZE_WITH_BORDER + i] =
                    source.data[(j - 1) * PATCH_SIZE_WITH_BORDER + (i - 1)] *
                        kernel[0][0] +
                    source.data[(j - 1) * PATCH_SIZE_WITH_BORDER + i] *
                        kernel[0][1] +

                    source.data[(j - 1) * PATCH_SIZE_WITH_BORDER + (i + 1)] *
                        kernel[0][2] +
                    source.data[(j + 1) * PATCH_SIZE_WITH_BORDER + (i - 1)] *
                        kernel[1][0] +

                    source.data[(j + 1) * PATCH_SIZE_WITH_BORDER + i] *
                        kernel[1][1] +
                    source.data[(j + 1) * PATCH_SIZE_WITH_BORDER + (i + 1)] *
                        kernel[1][2];
            }
        }
    }

    void dt(const Patch& first, const Patch& second, Patch& desitination) {
        desitination.origin.x = first.origin.x;
        desitination.origin.y = first.origin.y;

        for (int_fast32_t j = 0; j < PATCH_SIZE_WITH_BORDER; j++) {
            for (int_fast32_t i = 0; i < PATCH_SIZE_WITH_BORDER; i++) {
                desitination.data[j * PATCH_SIZE_WITH_BORDER + i] =
                    second.data[j * PATCH_SIZE_WITH_BORDER + i] -
                    first.data[j * PATCH_SIZE_WITH_BORDER + i];
            }
        }
    }

    void PatchPyramid::construct(image::ImagePyramid& image_pyramid,
                                 const image::KeyPoint* patch_centre_points,
                                 const size_t patch_centre_points_size) {

        if (patch_centre_points_size > MAX_NUMBER_OF_PATCHES_IN_PYRAMID_LEVEL) {

#ifdef CPU_MIMXRT1166DVM6A
            logger::errorf("Patch pyramid can't hold %d entries, max: %d\r\n",
                           patch_centre_points_size,
                           MAX_NUMBER_OF_PATCHES_IN_PYRAMID_LEVEL);
#else
            printf("Patch pyramid can't hold %lu entries, max: %d\r\n",
                   patch_centre_points_size,
                   MAX_NUMBER_OF_PATCHES_IN_PYRAMID_LEVEL);

#endif
            exit(1);
        }

        for (int pyramid_level = 0; pyramid_level < PYRAMID_LEVELS;
             pyramid_level++) {

            for (int patch_index = 0;
                 patch_index < (int_fast32_t)patch_centre_points_size;
                 patch_index++) {

                if (patch_centre_points[patch_index].stale) {
                    continue;
                }

                const int x =
                    (int_fast32_t)((patch_centre_points[patch_index].point.x -
                                    PATCH_SIZE / 2)) >>
                    pyramid_level;

                const int y =
                    (int_fast32_t)((patch_centre_points[patch_index].point.y -
                                    PATCH_SIZE / 2)) >>
                    pyramid_level;

                patches[patch_index][pyramid_level] =
                    Patch(x, y, *image_pyramid.at(pyramid_level));
            }
        }
    }
}
