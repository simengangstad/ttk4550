#include "clock.h"
#include "delay.h"

namespace delay {
    void ms(const uint32_t duration) {
        const uint32_t start = clock::ms();

        while (clock::ms() - start < duration) {}
    }
}
