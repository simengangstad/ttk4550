GROUP (
  "lib/nxp/sdk/lib/libgcc.a"
  "lib/nxp/sdk/lib/libc.a"
  "lib/nxp/sdk/lib/libstdc++.a"
  "lib/nxp/sdk/lib/libm.a"
  "lib/nxp/sdk/lib/libcr_newlib_none.a"
  "lib/nxp/sdk/lib/crti.o"
  "lib/nxp/sdk/lib/crtn.o"
  "lib/nxp/sdk/lib/crtbegin.o"
  "lib/nxp/sdk/lib/crtend.o"
)

MEMORY
{
  BOARD_FLASH (rx)          : ORIGIN = 0x30000000, LENGTH = 0x00FC0000 /* 16MB - 256 KB */
  CORE1_IMAGE (rx)          : ORIGIN = 0x30FC0000, LENGTH = 0x00040000 /* 256KB */
  SRAM_DTC_cm7 (rwx)        : ORIGIN = 0x20000000, LENGTH = 0x00070000 /* 448KB */
  SRAM_ITC_cm7 (rwx)        : ORIGIN = 0x00000000, LENGTH = 0x00010000 /* 64KB */
  rpmsg_sh_mem (rwx)        : ORIGIN = 0x20340000, LENGTH = 0x00002000 /* 8KB */
  SRAM_OC12 (rwx)           : ORIGIN = 0x20342000, LENGTH = 0x0001E000 /* 120KB */
  SRAM_OC3 (rwx)            : ORIGIN = 0x20360000, LENGTH = 0x00020000 /* 128KB */
  BOARD_SDRAM (rwx)         : ORIGIN = 0x80000000, LENGTH = 0x04000000 /* 64MB */
  NCACHE_REGION (rwx)       : ORIGIN = 0x83000000, LENGTH = 0x01000000 /* 16MB */
}
  /* Define a symbol for the top of each memory region */
  __base_BOARD_FLASH = 0x30000000  ; /* BOARD_FLASH */
  __base_Flash = 0x30000000 ; /* Flash */
  __top_BOARD_FLASH = 0x30000000 + 0x1000000 ; /* 16M bytes */
  __top_Flash = 0x30000000 + 0x1000000 ; /* 16M bytes */

  __base_SRAM_DTC_cm7 = 0x20000000  ; /* SRAM_DTC_cm7 */
  __base_RAM = 0x20000000 ; /* RAM */
  __top_SRAM_DTC_cm7 = 0x20000000 + 0x70000 ; /* 448K bytes */
  __top_RAM = 0x20000000 + 0x70000 ; /* 448K bytes */

  __base_SRAM_ITC_cm7 = 0x0  ; /* SRAM_ITC_cm7 */
  __base_RAM2 = 0x0 ; /* RAM2 */
  __top_SRAM_ITC_cm7 = 0x0 + 0x10000 ; /* 64K bytes */
  __top_RAM2 = 0x0 + 0x10000 ; /* 64K bytes */

  __base_rpmsg_sh_mem = 0x20340000  ;
  __top_rpmsg_sh_mem = 0x20340000 + 0x2000 ;

  __base_SRAM_OC12 = 0x20342000  ;
  __base_RAM3 = 0x20342000 ;
  __top_RAM3 = 0x20034200 + 0x1E000 ;

  __base_SRAM_OC3 = 0x20360000  ; /* SRAM_OC3 */
  __base_RAM5 = 0x20360000 ; /* RAM5 */
  __top_SRAM_OC3 = 0x20360000 + 0x20000 ; /* 128K bytes */
  __top_RAM5 = 0x20360000 + 0x20000 ; /* 128K bytes */

  __base_BOARD_SDRAM = 0x80000000  ; /* BOARD_SDRAM */
  __base_RAM6 = 0x80000000 ; /* RAM6 */
  __top_BOARD_SDRAM = 0x80000000 + 0x4000000 ; /* 64M bytes */
  __top_RAM6 = 0x80000000 + 0x4000000 ; /* 64M bytes */

  __base_NCACHE_REGION = 0x83000000  ; /* NCACHE_REGION */
  __base_RAM7 = 0x83000000 ; /* RAM7 */
  __top_NCACHE_REGION = 0x83000000 + 0x1000000 ; /* 16M bytes */
  __top_RAM7 = 0x83000000 + 0x1000000 ; /* 16M bytes */

ENTRY(ResetISR)

SECTIONS
{
     /* Image Vector Table and Boot Data for booting from external flash */
    .boot_hdr : ALIGN(4)
    {
        FILL(0xff)
        . = 0x400 ;
        __boot_hdr_start__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.conf))
        . = 0x1000 ;
        __boot_hdr_ivt_loadaddr__  =  ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.ivt))
        . = 0x1020 ;
        __boot_hdr_boot_data_loadaddr__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.boot_data))
        . = 0x1030 ;
        __boot_hdr_dcd_loadaddr__ = ABSOLUTE(.) ;
        KEEP(*(.boot_hdr.dcd_data))
        __boot_hdr_end__ = ABSOLUTE(.) ;
        . = 0x2000 ;
    } >BOARD_FLASH

    /* MAIN TEXT SECTION */
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM2));
        LONG(    ADDR(.data_RAM2));
        LONG(  SIZEOF(.data_RAM2));
        LONG(LOADADDR(.data_rpmsg_sh));
        LONG(    ADDR(.data_rpmsg_sh));
        LONG(  SIZEOF(.data_rpmsg_sh));
        LONG(LOADADDR(.data_RAM3));
        LONG(    ADDR(.data_RAM3));
        LONG(  SIZEOF(.data_RAM3));
        LONG(LOADADDR(.data_RAM5));
        LONG(    ADDR(.data_RAM5));
        LONG(  SIZEOF(.data_RAM5));
        LONG(LOADADDR(.data_RAM6));
        LONG(    ADDR(.data_RAM6));
        LONG(  SIZEOF(.data_RAM6));
        LONG(LOADADDR(.data_RAM7));
        LONG(    ADDR(.data_RAM7));
        LONG(  SIZEOF(.data_RAM7));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM2));
        LONG(  SIZEOF(.bss_RAM2));
        LONG(    ADDR(.bss_rpmsg_sh));
        LONG(  SIZEOF(.bss_rpmsg_sh));
        LONG(    ADDR(.bss_RAM3));
        LONG(  SIZEOF(.bss_RAM3));
        LONG(    ADDR(.bss_RAM5));
        LONG(  SIZEOF(.bss_RAM5));
        LONG(    ADDR(.bss_RAM6));
        LONG(  SIZEOF(.bss_RAM6));
        LONG(    ADDR(.bss_RAM7));
        LONG(  SIZEOF(.bss_RAM7));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */

        *(.after_vectors*)

       *(.text*)
       *(.rodata .rodata.* .constdata .constdata.*)
       . = ALIGN(4);
            /* C++ constructors etc */
            . = ALIGN(4);
            KEEP(*(.init))

            . = ALIGN(4);
            __preinit_array_start = .;
            KEEP (*(.preinit_array))
            __preinit_array_end = .;

            . = ALIGN(4);
            __init_array_start = .;
            KEEP (*(SORT(.init_array.*)))
            KEEP (*(.init_array))
            __init_array_end = .;

            KEEP(*(.fini));

            . = ALIGN(4);
            KEEP (*crtbegin.o(.ctors))
            KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
            KEEP (*(SORT(.ctors.*)))
            KEEP (*crtend.o(.ctors))

            . = ALIGN(4);
            KEEP (*crtbegin.o(.dtors))
            KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
            KEEP (*(SORT(.dtors.*)))
            KEEP (*crtend.o(.dtors))
            . = ALIGN(4);
            /* End C++ */
    } > BOARD_FLASH

    _etext = .;

    .core1_code :
    {
        . = ALIGN(4) ;
        KEEP (*(.core1_code))
        *(.core1_code*)
        . = ALIGN(4) ;
    } > CORE1_IMAGE


    /* DATA section for SRAM_ITC_cm7 */

    .data_RAM2 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM2 = .) ;
        PROVIDE(__start_data_SRAM_ITC_cm7 = .) ;
        *(.ramfunc.$RAM2)
        *(.ramfunc.$SRAM_ITC_cm7)
       KEEP(*(CodeQuickAccess))
       *(RamFunction)
        *(.data.$RAM2)
        *(.data.$SRAM_ITC_cm7)
        *(.data.$RAM2.*)
        *(.data.$SRAM_ITC_cm7.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM2 = .) ;
        PROVIDE(__end_data_SRAM_ITC_cm7 = .) ;
     } > SRAM_ITC_cm7 AT>BOARD_FLASH

    .data_rpmsg_sh : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_rpmsg_sh_mem = .) ;
        *(.ramfunc.$rpmsg_sh_mem)
        *(.data.$rpmsg_sh_mem)
        *(.data.$rpmsg_sh_mem.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_rpmsg_sh_mem = .) ;
     } > rpmsg_sh_mem AT>BOARD_FLASH


    /* DATA section for SRAM_OC12 */

    .data_RAM3 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM3 = .) ;
        PROVIDE(__start_data_SRAM_OC12 = .) ;
        *(.ramfunc.$RAM3)
        *(.ramfunc.$SRAM_OC12)
        *(.data.$RAM3)
        *(.data.$SRAM_OC12)
        *(.data.$RAM3.*)
        *(.data.$SRAM_OC12.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM3 = .) ;
        PROVIDE(__end_data_SRAM_OC12 = .) ;
     } > SRAM_OC12 AT>BOARD_FLASH

    /* DATA section for SRAM_OC3 */

    .data_RAM5 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM5 = .) ;
        PROVIDE(__start_data_SRAM_OC3 = .) ;
        *(.ramfunc.$RAM5)
        *(.ramfunc.$SRAM_OC3)
        *(.data.$RAM5)
        *(.data.$SRAM_OC3)
        *(.data.$RAM5.*)
        *(.data.$SRAM_OC3.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM5 = .) ;
        PROVIDE(__end_data_SRAM_OC3 = .) ;
     } > SRAM_OC3 AT>BOARD_FLASH

    /* DATA section for BOARD_SDRAM */

    .data_RAM6 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM6 = .) ;
        PROVIDE(__start_data_BOARD_SDRAM = .) ;
        *(.ramfunc.$RAM6)
        *(.ramfunc.$BOARD_SDRAM)
        *(.data.$RAM6)
        *(.data.$BOARD_SDRAM)
        *(.data.$RAM6.*)
        *(.data.$BOARD_SDRAM.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM6 = .) ;
        PROVIDE(__end_data_BOARD_SDRAM = .) ;
     } > BOARD_SDRAM AT>BOARD_FLASH

    /* DATA section for NCACHE_REGION */

    .data_RAM7 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM7 = .) ;
        PROVIDE(__start_data_NCACHE_REGION = .) ;
        *(.ramfunc.$RAM7)
        *(.ramfunc.$NCACHE_REGION)
        *(.data.$RAM7)
        *(.data.$NCACHE_REGION)
        *(.data.$RAM7.*)
        *(.data.$NCACHE_REGION.*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM7 = .) ;
        PROVIDE(__end_data_NCACHE_REGION = .) ;
     } > NCACHE_REGION AT>BOARD_FLASH

    /* MAIN DATA SECTION */
    .uninit_RESERVED (NOLOAD) : ALIGN(4)
    {
        _start_uninit_RESERVED = .;
        KEEP(*(.bss.$RESERVED*))
       . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > SRAM_DTC_cm7 AT> SRAM_DTC_cm7

    /* Main DATA section (SRAM_DTC_cm7) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       PROVIDE(__start_data_RAM = .) ;
       PROVIDE(__start_data_SRAM_DTC_cm7 = .) ;
       *(vtable)
       *(.ramfunc*)
       KEEP(*(DataQuickAccess))
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
       PROVIDE(__end_data_RAM = .) ;
       PROVIDE(__end_data_SRAM_DTC_cm7 = .) ;
    } > SRAM_DTC_cm7 AT>BOARD_FLASH

    /* BSS section for SRAM_ITC_cm7 */
    .bss_RAM2 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
       PROVIDE(__start_bss_SRAM_ITC_cm7 = .) ;
       *(.bss.$RAM2)
       *(.bss.$SRAM_ITC_cm7)
       *(.bss.$RAM2.*)
       *(.bss.$SRAM_ITC_cm7.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM2 = .) ;
       PROVIDE(__end_bss_SRAM_ITC_cm7 = .) ;
    } > SRAM_ITC_cm7 AT> SRAM_ITC_cm7


    /* BSS section for rpmsg_sh_mem */
    .bss_rpmsg_sh : ALIGN(4)
    {
       PROVIDE(__start_bss_rpmsg_sh = .) ;
       *(.bss.$rpmsg_sh_mem)
       *(.bss.$rpmsg_sh_mem.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_rpmsg_sh = .) ;
    } > rpmsg_sh_mem AT> rpmsg_sh_mem


    /* BSS section for SRAM_OC12 */
    .bss_RAM3 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM3 = .) ;
       PROVIDE(__start_bss_SRAM_OC12 = .) ;
       *(.bss.$RAM3)
       *(.bss.$SRAM_OC12)
       *(.bss.$RAM3.*)
       *(.bss.$SRAM_OC12.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM3 = .) ;
       PROVIDE(__end_bss_SRAM_OC12 = .) ;
    } > SRAM_OC12 AT> SRAM_OC12


    /* BSS section for SRAM_OC3 */
    .bss_RAM5 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM5 = .) ;
       PROVIDE(__start_bss_SRAM_OC3 = .) ;
       *(.bss.$RAM5)
       *(.bss.$SRAM_OC3)
       *(.bss.$RAM5.*)
       *(.bss.$SRAM_OC3.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM5 = .) ;
       PROVIDE(__end_bss_SRAM_OC3 = .) ;
    } > SRAM_OC3 AT> SRAM_OC3

    /* BSS section for BOARD_SDRAM */
    .bss_RAM6 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM6 = .) ;
       PROVIDE(__start_bss_BOARD_SDRAM = .) ;
       *(.bss.$RAM6)
       *(.bss.$BOARD_SDRAM)
       *(.bss.$RAM6.*)
       *(.bss.$BOARD_SDRAM.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM6 = .) ;
       PROVIDE(__end_bss_BOARD_SDRAM = .) ;
    } > BOARD_SDRAM AT> BOARD_SDRAM

    /* BSS section for NCACHE_REGION */
    .bss_RAM7 : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM7 = .) ;
       PROVIDE(__start_bss_NCACHE_REGION = .) ;
       *(.bss.$RAM7)
       *(.bss.$NCACHE_REGION)
       *(.bss.$RAM7.*)
       *(.bss.$NCACHE_REGION.*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM7 = .) ;
       PROVIDE(__end_bss_NCACHE_REGION = .) ;
    } > NCACHE_REGION AT> NCACHE_REGION

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        PROVIDE(__start_bss_RAM = .) ;
        PROVIDE(__start_bss_SRAM_DTC_cm7 = .) ;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(__end_bss_RAM = .) ;
        PROVIDE(__end_bss_SRAM_DTC_cm7 = .) ;
        PROVIDE(end = .);
    } > SRAM_DTC_cm7 AT> SRAM_DTC_cm7

    /* NOINIT section for SRAM_ITC_cm7 */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM2 = .) ;
       PROVIDE(__start_noinit_SRAM_ITC_cm7 = .) ;
       *(.noinit.$RAM2)
       *(.noinit.$SRAM_ITC_cm7)
       *(.noinit.$RAM2.*)
       *(.noinit.$SRAM_ITC_cm7.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM2 = .) ;
       PROVIDE(__end_noinit_SRAM_ITC_cm7 = .) ;
    } > SRAM_ITC_cm7 AT> SRAM_ITC_cm7

    /* NOINIT section for rpmsg_sh_mem */
    .noinit_rpmsg_sh (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_rpmsg_sh = .) ;
       *(.noinit.$rpmsg_sh_mem)
       *(.noinit.$rpmsg_sh_mem.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_rpmsg_sh = .) ;
    } > rpmsg_sh_mem AT> rpmsg_sh_mem

    /* NOINIT section for SRAM_OC1 */
    .noinit_RAM3 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM3 = .) ;
       PROVIDE(__start_noinit_SRAM_OC12 = .) ;
       *(.noinit.$RAM3)
       *(.noinit.$SRAM_OC12)
       *(.noinit.$RAM3.*)
       *(.noinit.$SRAM_OC12.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM3 = .) ;
       PROVIDE(__end_noinit_SRAM_OC12 = .) ;
    } > SRAM_OC12 AT> SRAM_OC12


    /* NOINIT section for SRAM_OC3 */
    .noinit_RAM5 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM5 = .) ;
       PROVIDE(__start_noinit_SRAM_OC3 = .) ;
       *(.noinit.$RAM5)
       *(.noinit.$SRAM_OC3)
       *(.noinit.$RAM5.*)
       *(.noinit.$SRAM_OC3.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM5 = .) ;
       PROVIDE(__end_noinit_SRAM_OC3 = .) ;
    } > SRAM_OC3 AT> SRAM_OC3

    /* NOINIT section for BOARD_SDRAM */
    .noinit_RAM6 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM6 = .) ;
       PROVIDE(__start_noinit_BOARD_SDRAM = .) ;
       *(.noinit.$RAM6)
       *(.noinit.$BOARD_SDRAM)
       *(.noinit.$RAM6.*)
       *(.noinit.$BOARD_SDRAM.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM6 = .) ;
       PROVIDE(__end_noinit_BOARD_SDRAM = .) ;
    } > BOARD_SDRAM AT> BOARD_SDRAM

    /* NOINIT section for NCACHE_REGION */
    .noinit_RAM7 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_noinit_RAM7 = .) ;
       PROVIDE(__start_noinit_NCACHE_REGION = .) ;
       *(.noinit.$RAM7)
       *(.noinit.$NCACHE_REGION)
       *(.noinit.$RAM7.*)
       *(.noinit.$NCACHE_REGION.*)
       . = ALIGN(4) ;
       PROVIDE(__end_noinit_RAM7 = .) ;
       PROVIDE(__end_noinit_NCACHE_REGION = .) ;
    } > NCACHE_REGION AT> NCACHE_REGION

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        PROVIDE(__start_noinit_RAM = .) ;
        PROVIDE(__start_noinit_SRAM_DTC_cm7 = .) ;
        *(.noinit*)
         . = ALIGN(4) ;
        _end_noinit = .;
       PROVIDE(__end_noinit_RAM = .) ;
       PROVIDE(__end_noinit_SRAM_DTC_cm7 = .) ;
    } > SRAM_DTC_cm7 AT> SRAM_DTC_cm7

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > BOARD_FLASH

    .ARM.exidx : ALIGN(4)
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > BOARD_FLASH

    /* Reserve and place Heap within memory map */
    _HeapSize = 0x4000000;
    .heap :  ALIGN(4)
    {
        _pvHeapStart = .;
        . += _HeapSize;
        . = ALIGN(4);
        _pvHeapLimit = .;
    } > BOARD_SDRAM

     _StackSize = 0x8000;
     /* Reserve space in memory for Stack */
    .heap2stackfill  :
    {
        . += _StackSize;
    } > SRAM_DTC_cm7
    /* Locate actual Stack in memory map */
    .stack ORIGIN(SRAM_DTC_cm7) + LENGTH(SRAM_DTC_cm7) - _StackSize - 0:  ALIGN(4)
    {
        _vStackBase = .;
        . = ALIGN(4);
        _vStackTop = . + _StackSize;
    } > SRAM_DTC_cm7

    /* Provide basic symbols giving location and size of main text
     * block, including initial values of RW data sections. Note that
     * these will need extending to give a complete picture with
     * complex images (e.g multiple Flash banks).
     */
    _image_start = LOADADDR(.text);
    _image_end = LOADADDR(.data) + SIZEOF(.data);
    _image_size = _image_end - _image_start;
    /* Provide symbols for MIMXRT1160 parts for boot header generation code
     * to set image to be plain load image or XIP.
     * Config : Plain load image = false
     */
    _boot_loadaddr = ORIGIN(BOARD_FLASH);
    _boot_size = LENGTH(BOARD_FLASH);
}
