#ifndef DATASET_LOADER_H_
#define DATASET_LOADER_H_

#include <stddef.h>
#include <stdint.h>

#include "image.h"

namespace dataset_loader {

    /**
     * @brief Sets the working directory for the dataset loader and retrieves
     * the file names for the data in the directory.
     */
    void initialise(const char* path);

    /**
     * @brief Frees up the memory used by the dataset loader.
     */
    void deinitialise();

    /**
     * @brief Retrieves the next image in the working directory.
     *
     * @param image Reference to the image, where the image data will be placed.
     * @param index Optional index, if set to -1, the next image in the
     * directory will be loaded.
     *
     * @return true if image was retrieved.
     */
    bool retrieve_image(image::Image& image, const int32_t index = -1);

}

#endif
