#ifndef FEATURE_EXTRACTION_H_
#define FEATURE_EXTRACTION_H_

#include <stddef.h>
#include <stdint.h>

#include "image.h"
#include "linalg.h"

namespace frontend {

    /**
     * @brief Performs FAST on a given image.
     *
     * @param image_buffer [in] Buffer for the image.
     * @param width [in] The width of the image.
     * @param height [in] The height of the image.
     * @param threshold [in] Threshold used for determining if a pixel is a
     * corner/feature or not.
     * @param out_keypoints [out] Features/corners detected are placed in
     * this buffer.
     * @param out_keypoints_size [out] Number of features/corners detected
     * are placed in this integer pointer.
     */
    void extract_features(const uint8_t* image_buffer,
                          const int_fast32_t width,
                          const int_fast32_t height,
                          const uint8_t threshold,
                          image::KeyPoint* out_keypoints,
                          uint32_t* out_keypoints_size);

}

#endif
