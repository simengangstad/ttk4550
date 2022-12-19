#ifndef TEST_lUCAS_KANADE
#define TEST_lUCAS_KANADE

#include "image.h"

namespace test {
    namespace lucas_kanade {

        void test_with_dataset(uint8_t* image_data_buffer,
                               uint8_t* image_pyramid_buffer,
                               image::PatchPyramid* patch_pyramid,
                               image::KeyPoint* keypoints_buffer,
                               image::KeyPoint* end_keypoints_buffer,
                               const size_t keypoints_buffer_size,
                               const char* dataset_name);

        void test_with_dataset_without_references_with_resample(
            uint8_t* image_data_buffer,
            uint8_t* image_pyramid_buffer,
            image::PatchPyramid* patch_pyramid,
            image::KeyPoint* keypoints_buffer,
            image::KeyPoint* end_keypoints_buffer,
            const size_t keypoints_buffer_size,
            const char* dataset_name,
            const size_t start_index,
            const size_t end_index);

    }

}

#endif
