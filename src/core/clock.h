#ifndef _CLOCK_CONFIG_H_
#define _CLOCK_CONFIG_H_

#include "board.h"
#include "fsl_common.h"

#ifdef __cplusplus

namespace clock {

    /**
     * @brief Initialises the main oscillators and clocks.
     */
    void initialise();

    /**
     * @return Milliseconds since boot.
     */
    uint32_t ms();

}

#endif

#endif
