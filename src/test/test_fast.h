#ifndef TEST_FAST_H
#define TEST_FAST_H

#include "image.h"

#include <stdint.h>

namespace test {

    namespace fast {

        void test_with_dataset(uint8_t* image_data_buffer,
                               image::KeyPoint* keypoints_buffer,
                               const size_t keypoints_buffer_size,
                               const char* dataset_name);

        void
        test_with_dataset_without_references(uint8_t* image_data_buffer,
                                             image::KeyPoint* keypoints_buffer,
                                             const size_t keypoints_buffer_size,
                                             const char* dataset_name);

    }
}

#endif
