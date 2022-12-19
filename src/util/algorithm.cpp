#include "algorithm.h"

#include <string.h>

namespace alg {
    static void swap(char** a, char** b) {
        char* temp = *a;
        *a         = *b;
        *b         = temp;
    }

    void quicksort(char** array, size_t length) {

        if (length <= 1) {
            return;
        }

        size_t pivot = 0;

        // Go through and partition into lower and upper segments, where the
        // pivot is the last entry in the array
        for (size_t i = 0; i < length; i++) {

            const size_t first_length  = strlen(array[i]);
            const size_t second_length = strlen(array[length - 1]);

            if (first_length < second_length ||
                ((first_length == second_length) &&
                 (strcmp(array[i], array[length - 1]) < 0))) {
                swap(array + i, array + pivot++);
            }
        }

        // At this point, the entries at the indices lower than pivot are all
        // "less" than the pivot. At this point the pivot is still at the end of
        // the array, so move it to the actual pivot index
        swap(array + pivot, array + length - 1);

        // Now recursively sort the lower partition and the upper partition
        quicksort(array, pivot++);
        quicksort(array + pivot, length - pivot);
    }
}
