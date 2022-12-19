#include "memory.h"

#include "fsl_device_registers.h"

#include <assert.h>
#include <stdlib.h>

#if __CORTEX_M == 7

static void mpu_init() {

    #if defined(__USE_SHMEM)
    extern uint32_t __base_rpmsg_sh_mem;
    extern uint32_t __top_rpmsg_sh_mem;
    uint32_t nonCacheStart = (uint32_t)(&__base_rpmsg_sh_mem);
    uint32_t size          = (uint32_t)(&__top_rpmsg_sh_mem) - nonCacheStart;
    #else
    extern uint32_t __base_NCACHE_REGION;
    extern uint32_t __top_NCACHE_REGION;
    uint32_t nonCacheStart = (uint32_t)(&__base_NCACHE_REGION);
    uint32_t size          = (uint32_t)(&__top_NCACHE_REGION) - nonCacheStart;
    #endif

    volatile uint32_t i = 0;

    #if defined(__ICACHE_PRESENT) && __ICACHE_PRESENT
    /* Disable I cache and D cache */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR)) {
        SCB_DisableICache();
    }
    #endif

    #if defined(__DCACHE_PRESENT) && __DCACHE_PRESENT
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR)) {
        SCB_DisableDCache();
    }
    #endif

    ARM_MPU_Disable();

    // clang-format off
		/* MPU configure:
		 * Use ARM_MPU_RASR(DisableExec, AccessPermission, TypeExtField, IsShareable, IsCacheable, IsBufferable,
		 * SubRegionDisable, Size)
		 * API in mpu_armv7.h.
		 * param DisableExec       Instruction access (XN) disable bit,0=instruction fetches enabled, 1=instruction fetches
		 * disabled.
		 * param AccessPermission  Data access permissions, allows you to configure read/write access for User and
		 * Privileged mode.
		 *      Use MACROS defined in mpu_armv7.h:
		 * ARM_MPU_AP_NONE/ARM_MPU_AP_PRIV/ARM_MPU_AP_URO/ARM_MPU_AP_FULL/ARM_MPU_AP_PRO/ARM_MPU_AP_RO
		 * Combine TypeExtField/IsShareable/IsCacheable/IsBufferable to configure MPU memory access attributes.
		 *  TypeExtField  IsShareable  IsCacheable  IsBufferable   Memory Attribute    Shareability        Cache
		 *     0             x           0           0             Strongly Ordered    shareable
		 *     0             x           0           1              Device             shareable
		 *     0             0           1           0              Normal             not shareable   Outer and inner write
		 * through no write allocate
		 *     0             0           1           1              Normal             not shareable   Outer and inner write
		 * back no write allocate
		 *     0             1           1           0              Normal             shareable       Outer and inner write
		 * through no write allocate
		 *     0             1           1           1              Normal             shareable       Outer and inner write
		 * back no write allocate
		 *     1             0           0           0              Normal             not shareable   outer and inner
		 * noncache
		 *     1             1           0           0              Normal             shareable       outer and inner
		 * noncache
		 *     1             0           1           1              Normal             not shareable   outer and inner write
		 * back write/read acllocate
		 *     1             1           1           1              Normal             shareable       outer and inner write
		 * back write/read acllocate
		 *     2             x           0           0              Device              not shareable
		 *  Above are normal use settings, if your want to see more details or want to config different inner/outter cache
		 * policy.
		 *  please refer to Table 4-55 /4-56 in arm cortex-M7 generic user guide <dui0646b_cortex_m7_dgug.pdf>
		 * param SubRegionDisable  Sub-region disable field. 0=sub-region is enabled, 1=sub-region is disabled.
		 * param Size              Region size of the region to be configured. use ARM_MPU_REGION_SIZE_xxx MACRO in
		 * mpu_armv7.h.
		 */

    // clang-format on

    /*
     * Add default region to deny access to whole address space to
     * workaround speculative prefetch. Refer to Arm errata 1013783-B for
     * more details.
     */

    /* Region 0 setting: Instruction access disabled, No data access
     * permission.
     */
    MPU->RBAR = ARM_MPU_RBAR(0, 0x00000000U);
    MPU->RASR = ARM_MPU_RASR(1,
                             ARM_MPU_AP_NONE,
                             0,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_4GB);

    /* Region 1 setting: Memory with Device type, not shareable,
     * non-cacheable.
     */
    MPU->RBAR = ARM_MPU_RBAR(1, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_512MB);

    /* Region 2 setting: Memory with Device type, not shareable,
     * non-cacheable.
     */
    MPU->RBAR = ARM_MPU_RBAR(2, 0x60000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_512MB);

    /* Region 3 setting: Memory with Device type, not shareable,
     * non-cacheable.
     */
    MPU->RBAR = ARM_MPU_RBAR(3, 0x00000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_1GB);

    /* Region 4 setting: Memory with Normal type, not shareable, outer/inner
     * write back */
    // ITCM
    MPU->RBAR = ARM_MPU_RBAR(4, 0x00000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             1,
                             0,
                             ARM_MPU_REGION_SIZE_64KB);

    /* Region 5 setting: Memory with Normal type, not shareable, outer/inner
     * write back */
    MPU->RBAR = ARM_MPU_RBAR(5, 0x20000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             1,
                             0,
                             ARM_MPU_REGION_SIZE_512KB);

    #if defined(CACHE_MODE_WRITE_THROUGH) && CACHE_MODE_WRITE_THROUGH
    /* Region 6 setting: Memory with Normal type, not shareable, write
     * through
     */
    MPU->RBAR = ARM_MPU_RBAR(6, 0x20200000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_1MB);

    /* Region 7 setting: Memory with Normal type, not shareable, write
     * trough */
    MPU->RBAR = ARM_MPU_RBAR(7, 0x20300000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_512KB);
    #else
    /* Region 6 setting: Memory with Normal type, not shareable, outer/inner
     * write back */
    MPU->RBAR = ARM_MPU_RBAR(6, 0x20200000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             1,
                             0,
                             ARM_MPU_REGION_SIZE_1MB);

    /* Region 7 setting: Memory with Normal type, not shareable, outer/inner
     * write back */
    MPU->RBAR = ARM_MPU_RBAR(7, 0x20300000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             1,
                             0,
                             ARM_MPU_REGION_SIZE_512KB);
    #endif

    #if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
    /* Region 8 setting: Memory with Normal type, not shareable, outer/inner
     * write back. */
    MPU->RBAR = ARM_MPU_RBAR(8, 0x30000000U);
    MPU->RASR =
        ARM_MPU_RASR(0, ARM_MPU_AP_RO, 0, 0, 1, 1, 0, ARM_MPU_REGION_SIZE_16MB);
    #endif

    #ifdef USE_SDRAM

        #if defined(CACHE_MODE_WRITE_THROUGH) && CACHE_MODE_WRITE_THROUGH
    /* Region 9 setting: Memory with Normal type, not shareable, write
     * trough */
    MPU->RBAR = ARM_MPU_RBAR(9, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_64MB);
        #else
    /* Region 9 setting: Memory with Normal type, not shareable, outer/inner
     * write back */
    MPU->RBAR = ARM_MPU_RBAR(9, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             1,
                             0,
                             ARM_MPU_REGION_SIZE_64MB);
        #endif
    #endif

    while ((size >> i) > 0x1U) { i++; }

    if (i != 0) {
        /* The MPU region size should be 2^N, 5<=N<=32, region base should
         * be multiples of size. */
        assert(!(nonCacheStart % size));
        assert(size == (uint32_t)(1 << i));
        assert(i >= 5);

        /* Region 10 setting: Memory with Normal type, not shareable,
         * non-cacheable */
        MPU->RBAR = ARM_MPU_RBAR(10, nonCacheStart);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, i - 1);
    }

    /* Region 11 setting: Memory with Device type, not shareable,
     * non-cacheable
     */
    MPU->RBAR = ARM_MPU_RBAR(11, 0x40000000);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_16MB);

    /* Region 12 setting: Memory with Device type, not shareable,
     * non-cacheable
     */
    MPU->RBAR = ARM_MPU_RBAR(12, 0x41000000);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_2MB);

    /* Region 13 setting: Memory with Device type, not shareable,
     * non-cacheable
     */
    MPU->RBAR = ARM_MPU_RBAR(13, 0x41400000);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_1MB);

    /* Region 14 setting: Memory with Device type, not shareable,
     * non-cacheable
     */
    MPU->RBAR = ARM_MPU_RBAR(14, 0x41800000);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_2MB);

    /* Region 15 setting: Memory with Device type, not shareable,
     * non-cacheable
     */
    MPU->RBAR = ARM_MPU_RBAR(15, 0x42000000);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             2,
                             0,
                             0,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_1MB);

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    #if defined(__DCACHE_PRESENT) && __DCACHE_PRESENT
    SCB_EnableDCache();
    #endif

    #if defined(__ICACHE_PRESENT) && __ICACHE_PRESENT
    SCB_EnableICache();
    #endif
}

#elif __CORTEX_M == 4

void mpu_init() {

    extern uint32_t __base_NCACHE_REGION;
    extern uint32_t __top_NCACHE_REGION;
    uint32_t nonCacheStart = (uint32_t)(&__base_NCACHE_REGION);
    uint32_t nonCacheSize  = (uint32_t)(&__top_NCACHE_REGION) - nonCacheStart;

    #if defined(__USE_SHMEM)
    extern uint32_t __base_rpmsg_sh_mem;
    extern uint32_t __top_rpmsg_sh_mem;
    uint32_t rpmsgShmemStart = (uint32_t)(&__base_rpmsg_sh_mem);
    uint32_t rpmsgShmemSize = (uint32_t)(&__top_rpmsg_sh_mem) - rpmsgShmemStart;
    #endif

    uint32_t i = 0;

    /* Only config non-cacheable region on system bus */
    assert(nonCacheStart >= 0x20000000);

    /* Disable code bus cache */
    if (LMEM_PCCCR_ENCACHE_MASK == (LMEM_PCCCR_ENCACHE_MASK & LMEM->PCCCR)) {
        /* Enable the processor code bus to push all modified lines. */
        LMEM->PCCCR |= LMEM_PCCCR_PUSHW0_MASK | LMEM_PCCCR_PUSHW1_MASK |
                       LMEM_PCCCR_GO_MASK;
        /* Wait until the cache command completes. */
        while ((LMEM->PCCCR & LMEM_PCCCR_GO_MASK) != 0U) {}
        /* As a precaution clear the bits to avoid inadvertently re-running
         * this command. */
        LMEM->PCCCR &= ~(LMEM_PCCCR_PUSHW0_MASK | LMEM_PCCCR_PUSHW1_MASK);
        /* Now disable the cache. */
        LMEM->PCCCR &= ~LMEM_PCCCR_ENCACHE_MASK;
    }

    /* Disable system bus cache */
    if (LMEM_PSCCR_ENCACHE_MASK == (LMEM_PSCCR_ENCACHE_MASK & LMEM->PSCCR)) {
        /* Enable the processor system bus to push all modified lines. */
        LMEM->PSCCR |= LMEM_PSCCR_PUSHW0_MASK | LMEM_PSCCR_PUSHW1_MASK |
                       LMEM_PSCCR_GO_MASK;
        /* Wait until the cache command completes. */
        while ((LMEM->PSCCR & LMEM_PSCCR_GO_MASK) != 0U) {}
        /* As a precaution clear the bits to avoid inadvertently re-running
         * this command. */
        LMEM->PSCCR &= ~(LMEM_PSCCR_PUSHW0_MASK | LMEM_PSCCR_PUSHW1_MASK);
        /* Now disable the cache. */
        LMEM->PSCCR &= ~LMEM_PSCCR_ENCACHE_MASK;
    }

    /* Disable MPU */
    ARM_MPU_Disable();

    #if defined(CACHE_MODE_WRITE_THROUGH) && CACHE_MODE_WRITE_THROUGH
    /* Region 0 setting: Memory with Normal type, not shareable, write
     * trough */
    MPU->RBAR = ARM_MPU_RBAR(0, 0x20200000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_1MB);

    /* Region 1 setting: Memory with Normal type, not shareable, write
     * through
     */
    MPU->RBAR = ARM_MPU_RBAR(1, 0x20300000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_512KB);

    /* Region 2 setting: Memory with Normal type, not shareable, write
     * through
     */
    MPU->RBAR = ARM_MPU_RBAR(2, 0x80000000U);
    MPU->RASR = ARM_MPU_RASR(0,
                             ARM_MPU_AP_FULL,
                             0,
                             0,
                             1,
                             0,
                             0,
                             ARM_MPU_REGION_SIZE_64MB);

    while ((nonCacheSize >> i) > 0x1U) { i++; }

    if (i != 0) {
        /* The MPU region size should be 2^N, 5<=N<=32, region base should
         * be multiples of size. */
        assert(!(nonCacheStart % nonCacheSize));
        assert(nonCacheSize == (uint32_t)(1 << i));
        assert(i >= 5);

        /* Region 3 setting: Memory with device type, not shareable,
         * non-cacheable */
        MPU->RBAR = ARM_MPU_RBAR(3, nonCacheStart);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, i - 1);
    }

        #if defined(__USE_SHMEM)
    i = 0;

    while ((rpmsgShmemSize >> i) > 0x1U) { i++; }

    if (i != 0) {
        /* The MPU region size should be 2^N, 5<=N<=32, region base should
         * be multiples of size. */
        assert(!(rpmsgShmemStart % rpmsgShmemSize));
        assert(rpmsgShmemSize == (uint32_t)(1 << i));
        assert(i >= 5);

        /* Region 4 setting: Memory with device type, not shareable,
         * non-cacheable */
        MPU->RBAR = ARM_MPU_RBAR(4, rpmsgShmemStart);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, i - 1);
    }
        #endif
    #else
    while ((nonCacheSize >> i) > 0x1U) { i++; }

    if (i != 0) {
        /* The MPU region size should be 2^N, 5<=N<=32, region base should
         * be multiples of size. */
        assert(!(nonCacheStart % nonCacheSize));
        assert(nonCacheSize == (uint32_t)(1 << i));
        assert(i >= 5);

        /* Region 0 setting: Memory with device type, not shareable,
         * non-cacheable */
        MPU->RBAR = ARM_MPU_RBAR(0, nonCacheStart);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, i - 1);
    }

        #if defined(__USE_SHMEM)
    i = 0;

    while ((rpmsgShmemSize >> i) > 0x1U) { i++; }

    if (i != 0) {
        /* The MPU region size should be 2^N, 5<=N<=32, region base should
         * be multiples of size. */
        assert(!(rpmsgShmemStart % rpmsgShmemSize));
        assert(rpmsgShmemSize == (uint32_t)(1 << i));
        assert(i >= 5);

        /* Region 1 setting: Memory with device type, not shareable,
         * non-cacheable */
        MPU->RBAR = ARM_MPU_RBAR(1, rpmsgShmemStart);
        MPU->RASR = ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 2, 0, 0, 0, 0, i - 1);
    }
        #endif
    #endif

    /* Enable MPU */
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

    /* Enables the processor system bus to invalidate all lines in both
    ways. and Initiate the processor system bus cache command. */
    LMEM->PSCCR |= LMEM_PSCCR_INVW0_MASK | LMEM_PSCCR_INVW1_MASK |
                   LMEM_PSCCR_GO_MASK;
    /* Wait until the cache command completes */
    while ((LMEM->PSCCR & LMEM_PSCCR_GO_MASK) != 0U) {}
    /* As a precaution clear the bits to avoid inadvertently re-running this
     * command. */
    LMEM->PSCCR &= ~(LMEM_PSCCR_INVW0_MASK | LMEM_PSCCR_INVW1_MASK);
    /* Now enable the system bus cache. */
    LMEM->PSCCR |= LMEM_PSCCR_ENCACHE_MASK;

    /* Enables the processor code bus to invalidate all lines in both ways.
    and Initiate the processor code bus code cache command. */
    LMEM->PCCCR |= LMEM_PCCCR_INVW0_MASK | LMEM_PCCCR_INVW1_MASK |
                   LMEM_PCCCR_GO_MASK;
    /* Wait until the cache command completes. */
    while ((LMEM->PCCCR & LMEM_PCCCR_GO_MASK) != 0U) {}
    /* As a precaution clear the bits to avoid inadvertently re-running this
     * command. */
    LMEM->PCCCR &= ~(LMEM_PCCCR_INVW0_MASK | LMEM_PCCCR_INVW1_MASK);
    /* Now enable the code bus cache. */
    LMEM->PCCCR |= LMEM_PCCCR_ENCACHE_MASK;
}
#endif

/**
 * @brief Defined in linker file.
 */
extern const uint32_t _pvHeapLimit;

extern const uint32_t _HeapSize;

extern uint32_t __end_of_heap;

extern const uint32_t _vStackBase;

namespace memory {
    void configure_access_policy() { mpu_init(); }

    int64_t available_on_heap() {

        if (__end_of_heap == 0) {
            return (uint32_t)&_HeapSize;
        }

        return (int64_t)((uint32_t)&_pvHeapLimit) -
               (int64_t)((uint32_t)__end_of_heap);
    }

    int64_t available_on_stack() {
        void* sp;
        __asm volatile("mov %0, sp" : "=r"(sp));

        int64_t available = ((int64_t)((uint32_t)sp) -
                             (int64_t)(((uint32_t)&_vStackBase)));

        return available;
    }
}
