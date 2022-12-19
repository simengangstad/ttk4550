#ifndef PROFILE_H_
#define PROFILE_H_

#include "clock.h"
#include "logger.h"

#include <stdbool.h>
#include <stdio.h>

#include "fsl_device_registers.h"

#define PROFILE_START()                                                  \
    {                                                                    \
        logger::debugf("%s:%d - Profile start\r\n", __FILE__, __LINE__); \
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;                  \
        DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;                            \
        DWT->CYCCNT = 0UL;                                               \
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                             \
    }

#define PROFILE_END()                                                              \
    {                                                                              \
        DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;                                      \
        uint32_t cycles = DWT->CYCCNT;                                             \
        float ms        = 1000.0f * (float)((float)cycles /                        \
                                     (float)BOARD_BOOTCLOCKRUN_CORE_CLOCK); \
                                                                                   \
        logger::debugf("%s:%d - Profile end, took %f ms\r\n",                      \
                       __FILE__,                                                   \
                       __LINE__,                                                   \
                       ms);                                                        \
    }

#endif
