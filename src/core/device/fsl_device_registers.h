#ifndef __FSL_DEVICE_REGISTERS_H__
#define __FSL_DEVICE_REGISTERS_H__

/*
 * Include the cpu specific register header files.
 *
 * The CPU macro should be declared in the project or makefile.
 */
#if (defined(CPU_MIMXRT1166CVM5A_cm7) || defined(CPU_MIMXRT1166DVM6A_cm7) || \
     defined(CPU_MIMXRT1166XVM5A_cm7))

#define MIMXRT1166_cm7_SERIES

// CMSIS-style register definitions
#include "MIMXRT1166_cm7.h"

// CPU specific feature definitions
#include "MIMXRT1166_cm7_features.h"

#elif (defined(CPU_MIMXRT1166CVM5A_cm4) || defined(CPU_MIMXRT1166DVM6A_cm4) || \
       defined(CPU_MIMXRT1166XVM5A_cm4))

#define MIMXRT1166_cm4_SERIES

// CMSIS-style register definitions
#include "MIMXRT1166_cm4.h"
// CPU specific feature definitions
#include "MIMXRT1166_cm4_features.h"

#else
#error "No valid CPU defined!"
#endif

#endif
