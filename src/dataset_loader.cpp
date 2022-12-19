#include "dataset_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#include "file_system.h"
#include "lodepng.h"
#include "logger.h"

#include "algorithm.h"

namespace dataset_loader {

    /**
     * @brief Index to the next file which will be loaded.
     */
    static size_t current_file_index = 1;

    void initialise(const char* path) {

        current_file_index = 1;

        deinitialise();

        file_system::cd(path);
    }

    void deinitialise() {}

    bool retrieve_image(image::Image& image, const int32_t index) {

        // Retrieve next element in directory
        char file_path[16] = "";

        if (index == -1) {
            sprintf(file_path, "%u.png", current_file_index++);
        } else {
            sprintf(file_path, "%ld.png", index);
            current_file_index = index + 1;
        }

        // Retrieve the file size
        uint32_t file_size = 0;
        if (!file_system::size(file_path, &file_size)) {
            return false;
        }

        // The size of the read/write buffer should be a multiple of 512,
        // since SDHC/SDXC card uses 512-byte fixed block length
        const uint32_t file_content_size = ((file_size / 512) + 1) * 512;

        uint8_t* file_content_buffer = (uint8_t*)malloc(file_content_size);

        if (file_content_buffer == NULL) {
            logger::errorf("File content allocation failed\r\r");
            return false;
        }

        if (!file_system::read(file_path, file_content_buffer, file_size)) {
            free(file_content_buffer);
            return false;
        }

        // Now we decode the PNG
        uint8_t decode_status = lodepng_decode_memory(&image.data,
                                                      &image.width,
                                                      &image.height,
                                                      file_content_buffer,
                                                      file_size,
                                                      LCT_GREY,
                                                      8);

        // Free the file content buffer now that we have the PNG data
        free(file_content_buffer);

        if (decode_status != 0) {
            logger::errorf(
                "Error happened while decoding PNG. Error code: %d\r\n",
                decode_status);
            return false;
        }

        return true;
    }
}
