#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "linalg.h"

#ifdef CPU_MIMXRT1166DVM6A
    #include "profile.h"
#endif

constexpr uint16_t PYRAMID_LEVELS = 5;
constexpr uint16_t PATCH_SIZE     = 7;

constexpr uint16_t PATCH_SIZE_WITH_BORDER                 = PATCH_SIZE + 2;
constexpr uint16_t MAX_NUMBER_OF_PATCHES_IN_PYRAMID_LEVEL = 74;

namespace image {

    /**
     * @brief Struct for a 8 bit, 1 channel grayscale image.
     */
    struct Image {
        /**
         * @brief Buffer of the image data. Note that this has to be passed to
         * the image, the image struct in itself does not allocate any memory.
         */
        uint8_t* data = NULL;

        /**
         * @brief Width of the image.
         */
        size_t width = 0;

        /**
         * @brief Height of the image.
         */
        size_t height = 0;

        /**
         * @brief Initializes an empty image.
         */
        Image() : data(NULL), width(0), height(0) {}

        /**
         * @brief Initializes the image with a data buffer and a resolution. No
         * copy will be made here, so operations on the data will modify the
         * original buffer passed to this image.
         *
         * @param data Pointer to the image buffer.
         * @param image_width Width of the image.
         * @param image_height Height of the image.
         */
        Image(uint8_t* data_buffer,
              const size_t image_width,
              const size_t image_height)
            : data(data_buffer), width(image_width), height(image_height) {}
    };

    /**
     * @brief Blurs a image data buffer inplace.
     *
     * @param image_data The image data buffer.
     * @param image_width Width of image.
     * @param image_height Height of image.
     */
    void
    blur_inplace(uint8_t* image_data, size_t image_width, size_t image_height);

    /**
     * @brief Blurs an image inplace, will this override the existing data
     * in the @p image.
     *
     * @param image The image to blur.
     */
    void blur_inplace(Image& image);

    /**
     * @brief Downsamples the image in half for each axis, and does it
     * inplace. Thus, this will modify the buffer of @p source.
     *
     * @param source The image to downscale.
     */
    void downsample_inplace(Image& source);

    /**
     * @brief Downsamples the image in half for each axis and places the
     * result in @p destination.
     *
     * @param source The image to downscale.
     * @param destination Where the downscaled image is placed.
     */
    void downsample(const Image& source, Image& destination);

    /**
     * @brief A pyramid consisting of images, each downsampled in half for
     * each axis for each level.
     */
    struct ImagePyramid {

      private:
        /**
         * @brief The images in the pyramid level.
         */
        Image images[PYRAMID_LEVELS] = {};

      public:
        /**
         * @brief Constructs an image pyramid from a @p image. Note that the
         * first level in the pyramid will refer to the image passed to this
         * constructor. The rest of the images' buffers will be placed in
         * the pyramid buffer, which has to be preallocated and made sure to
         * have enough space to fit the images in the pyramid.
         *
         * @param source The source image in the first level of the pyramid.
         * @param pyramid_buffer Where the rest of the images' data will be
         * placed (above the first level).
         */
        ImagePyramid(const Image& source, uint8_t* pyramid_buffer);

        /**
         * @return A pointer to the image at the given @p pyramid_level.
         */
        image::Image* at(const size_t pyramid_level);
    };

    struct Patch;

    /**
     * @brief Takes the x gradient of @p source and places it in @p
     * destination with use of convolution using a Sobel kernel.
     */
    void dx(const Patch& source, Patch& destination);

    /**
     * @brief Takes the y gradient of @p source and places it in @p
     * destination with use of convolution using a Sobel kernel.
     */
    void dy(const Patch& source, Patch& destination);

    /**
     * @brief Takes the time gradient from @p first and @p second and places
     * it in @p destination.
     */
    void dt(const Patch& first, const Patch& second, Patch& destination);

    /**
     * @brief A patch of a given image.
     *
     * @note The data of the patch is stored in floats, in order to make
     * gradients more precise.
     */
    struct Patch {
      private:
        /**
         * @brief The data of the patch. Note that we store a border around
         * the patch in order e.g. be able to take the gradient of the whole
         * patch (excluding the border), without having omit pixels.
         */
        float data[PATCH_SIZE_WITH_BORDER * PATCH_SIZE_WITH_BORDER];

      public:
        /**
         * @brief The upper left start point of the patch. Note that is is
         * not from the border, but the start of the actual patch.
         */
        linalg::Vec2 origin;

        /**
         * @brief Initializes the patch with empty data.
         */
        Patch() : origin{0, 0} { memset(data, 0, sizeof(data)); }

        /**
         * @brief Initializes the Patch with a start point and a reference
         * to the image where the data should be extracted from. Applies
         * bilinear filtering.
         *
         * @param x Start position of patch in x direction.
         * @param y Start position of patch in y direction.
         * @param image Where to grab the values for the patch from.
         */
        Patch(const float x, const float y, const image::Image& image);

        /**
         * @return A pointer to the @p index row. The pointer will point to
         * the value after the border at each row.
         */
        float* row(int index);

        friend void dx(const Patch& source, Patch& destination);
        friend void dy(const Patch& source, Patch& destination);
        friend void
        dt(const Patch& first, const Patch& second, Patch& destination);

        void print();
    };

    struct KeyPoint {
        linalg::Vec2 point;
        bool stale;

        KeyPoint() {
            point.x = 0;
            point.y = 0;
            stale   = false;
        }

        KeyPoint(float x, float y) {
            point.x = x;
            point.y = y;
            stale   = false;
        }
    };

    /**
     * @brief Contains N pyramids of patches, where each level in the
     * pyramid is downsampled by half for each axis.
     */
    struct PatchPyramid {
        /**
         * The patches in the pyramid.
         */
        image::Patch patches[MAX_NUMBER_OF_PATCHES_IN_PYRAMID_LEVEL]
                            [PYRAMID_LEVELS];

        /**
         * @brief Constructs a patch pyramid with N pyramids equal to the
         * amount of features and M pyramid levels.
         *
         * @note This will downsample the @p image inplace, and thus modify
         * it.
         *
         * @param image_pyramid The pyramid to construct the patches from.
         * Assumes that the first image (at the finest level) is blurred,
         * but the rest are not.
         * @param patch_centre_points The center points for each patch.
         * @param patch_centre_points_size Size of the patch start points.
         */
        void construct(image::ImagePyramid& image_pyramid,
                       const image::KeyPoint* patch_centre_points,
                       const size_t patch_centre_points_size);
    };
}

#endif
