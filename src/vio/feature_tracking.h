#ifndef FEATURE_TRACKING_H
#define FEATURE_TRACKING_H

#include "image.h"
#include "linalg.h"

#include <stddef.h>

namespace frontend {

    /**
     * @brief Tracks keypoints from a image to another.
     *
     * @param previous_patch_pyramid The previous patch pyramid where the
     * which contains patches around the keypoints.
     * @param next_image_pyramid The pyramid of the image where the keypoints
     * are to be found.
     * @param previous_keypoints The keypoints captured in the previous frame.
     * @param next_keypoints Buffer for where the keypoints found in @p
     * next_image are placed after tracking.
     * @param previous_keypoints_size Size of the keypoints buffer.
     */
    void track_features(image::PatchPyramid& previous_patch_pyramid,
                        image::ImagePyramid& next_image_pyramid,
                        image::KeyPoint* previous_keypoints,
                        image::KeyPoint* next_keypoints,
                        const size_t previous_keypoints_size);
}

#endif
