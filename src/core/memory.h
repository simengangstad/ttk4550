#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

namespace memory {
    /**
     * @brief Sets up the MPU with the rules for memory access.
     */
    void configure_access_policy();

    /**
     * @return Currently avaiable memory on heap. Will return the end of the
     * heap pointer, thus will not represent the exact amount of heap avaiable,
     * but an upper boundary.
     */
    int64_t available_on_heap();

    /**
     * @return Currently avaiable memory on stack. If overflow occurs, it will
     * return a negative number.
     */
    int64_t available_on_stack();
}

#endif
