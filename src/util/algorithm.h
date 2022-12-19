#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stddef.h>

namespace alg {

    /**
     * @brief Performs quicksort on strings inplace.
     *
     * @param array The array of strings.
     * @param length The length of @p array.
     */
    void quicksort(char** array, size_t length);
}

#endif
