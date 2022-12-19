#include "feature_tracking.h"

#include <math.h>

#ifdef CPU_MIMXRT1166DVM6A
    #include "board.h"
#else
    #define SECTION_ITCM
#endif

namespace frontend {

    void track_features(image::PatchPyramid& previous_patch_pyramid,
                        image::ImagePyramid& next_image_pyramid,
                        image::KeyPoint* previous_keypoints,
                        image::KeyPoint* next_keypoints,
                        const size_t previous_keypoints_size) {

        // This stores the flow computed for each feature at every pyramid
        // level. When going down in the pyramid, the flow from the previous
        // level will be upsampled (by 2) and used in the current level.
        linalg::Vec2 pyramid_level_flow[previous_keypoints_size]
                                       [PYRAMID_LEVELS];

        for (int pyramid_level = PYRAMID_LEVELS - 1; pyramid_level >= 0;
             pyramid_level--) {

            image::Image* next_image_at_pyramid_level = next_image_pyramid.at(
                pyramid_level);

            for (int_fast32_t feature_index = 0;
                 feature_index < (int_fast32_t)previous_keypoints_size;
                 feature_index++) {

                if (previous_keypoints[feature_index].stale) {
                    next_keypoints[feature_index].stale = true;
                    continue;
                }

                image::Patch previous_image_patch =
                    previous_patch_pyramid
                        .patches[feature_index][pyramid_level];

                image::Patch previous_patch_dx, previous_patch_dy;
                image::dx(previous_image_patch, previous_patch_dx);
                image::dy(previous_image_patch, previous_patch_dy);

                // LK is based on the following:
                //
                // A * v = b
                //
                // Where A contains the x and y gradients from the previous
                // image, v is the flow vector and b is the time derivatives
                // with respect to the previous image and the current image
                //
                // The equations is solved by taking the pseudo-inverse:
                //
                // A * v        = b
                // A^T * A * v  = A^T * b
                // v            = (A^T * A)^-1 * A^T * b
                // v            = H^-1 * A^T * b
                //
                // Where we let H = A^T * A
                //
                // A will thus be on the following form:
                //
                //     [ I0x(p_1) I0y(p_1) ]
                // A = [ I0x(p_2) I0y(p_2) ]
                //     [       ...         ]
                //     [ I0x(p_n) I0y(p_n) ]
                //
                // Whereas b will be on the following form (the time derivative
                // is equal to the intensity difference between the images)
                //
                //     [I1(p_1) - I0(p_1)]
                // b = [I1(p_2) - I0(p_2)]
                //     [       ...       ]
                //     [I1(p_n) - I0(p_n)]
                //
                // clang-format off
                //
                // This can be rewritten as:
                //
                //     [ Sum(I0x(p_i)^2)            Sum(I0x(p_i) * I0y(p_i)) ]^-1  [ -Sum(I0x(p_i) * (I1(p_i) - I0(p_i))) ]
                // v = [                                                     ]     [                                      ]
                //     [ Sum(I0x(p_i) * I0y(p_i))   Sum(I0y(p_i)^2)          ]     [ -Sum(I0y(p_i) * (I1(p_i) - I0(p_i))) ]
                //
                // clang-format on

                linalg::Mat<2 * 2> S(2, 2);

                linalg::Mat<PATCH_SIZE * PATCH_SIZE * 2> AT(2,
                                                            PATCH_SIZE *
                                                                PATCH_SIZE);

                linalg::Mat<PATCH_SIZE * PATCH_SIZE> b(PATCH_SIZE * PATCH_SIZE,
                                                       1);

                linalg::Mat<2> ATb(2, 1);

                for (int_fast32_t j = 0; j < PATCH_SIZE; j++) {
                    for (int_fast32_t i = 0; i < PATCH_SIZE; i++) {

                        const float Ix = previous_patch_dx.row(j)[i];
                        const float Iy = previous_patch_dy.row(j)[i];

                        S(0, 0) += Ix * Ix;
                        S(1, 1) += Iy * Iy;

                        S(0, 1) += Ix * Iy;

                        AT(0, j * PATCH_SIZE + i) = Ix;
                        AT(1, j * PATCH_SIZE + i) = Iy;
                    }
                }

                // Off-diagonal entries are equal
                S(1, 0) = S(0, 1);

                linalg::Mat<2 * 2> Sinv(2, 2);
                linalg::inverse(S, Sinv);

                const float S_determinant = linalg::determinant(S);

                if (isnan(S_determinant) || isinf(S_determinant)) {
                    printf("\tH is non-invertible!\r\n");
                    break;
                }

                // Set norm to max value before the minimization is performed
                float norm = FLT_MAX;

                int_fast32_t iterations = 0;

                linalg::Vec2 flow;

                linalg::Mat<2> incremental_flow;

                const float threshold = 0.01;

                do {

                    const linalg::Vec2 total_flow =
                        flow + pyramid_level_flow[feature_index][pyramid_level];

                    image::Patch next_image_patch(
                        previous_image_patch.origin.x + total_flow.x,
                        previous_image_patch.origin.y + total_flow.y,
                        *next_image_at_pyramid_level);

                    image::Patch patch_dt;
                    image::dt(previous_image_patch, next_image_patch, patch_dt);

                    float current_norm = 0;

                    for (int_fast32_t j = 0; j < PATCH_SIZE; j++) {
                        for (int_fast32_t i = 0; i < PATCH_SIZE; i++) {
                            const float It = patch_dt.row(j)[i];

                            current_norm += fabs(It);

                            b(j * PATCH_SIZE + i, 0) = -It;
                        }
                    }

                    linalg::multiply(AT, b, ATb);
                    linalg::multiply(Sinv, ATb, incremental_flow);

                    if (current_norm < norm) {
                        norm = current_norm;
                        flow.x += incremental_flow(0, 0);
                        flow.y += incremental_flow(1, 0);
                    } else {
                        break;
                    }

                } while (linalg::norm(incremental_flow) > threshold &&
                         iterations++ < 50);

                linalg::Vec2 pyramid_level_displacement =
                    flow + pyramid_level_flow[feature_index][pyramid_level];

                if (pyramid_level > 0) {

                    pyramid_level_flow[feature_index][pyramid_level - 1] =
                        2 * pyramid_level_displacement;

                } else {

                    image::KeyPoint& key_point = next_keypoints[feature_index];

                    key_point.point.x = (int)round(
                        previous_image_patch.origin.x + PATCH_SIZE / 2 +
                        pyramid_level_displacement.x);

                    key_point.point.y = (int)round(
                        previous_image_patch.origin.y + PATCH_SIZE / 2 +
                        pyramid_level_displacement.y);

                    if ((key_point.point.x < 0) || (key_point.point.y < 0) ||
                        (key_point.point.x >
                         next_image_pyramid.at(0)->width - 1) ||
                        (key_point.point.y >
                         next_image_pyramid.at(0)->height - 1)) {
                        key_point.stale = true;
                    } else {
                        key_point.stale = false;
                    }
                }
            }
        }
    }
}
