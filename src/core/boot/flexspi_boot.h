#ifndef __FLEXSPI_NOR_BOOT_H__
#define __FLEXSPI_NOR_BOOT_H__

#include <stdbool.h>
#include <stdint.h>

#include "fsl_common.h"

#define BOARD_FLASH_SIZE (0x1000000U)

/**
 * @brief FLEXSPI memory config block related definitions
 */
#define FLEXSPI_CFG_BLK_TAG     (0x42464346UL) // ascii "FCFB" Big Endian
#define FLEXSPI_CFG_BLK_VERSION (0x56010400UL) // V1.4.0
#define FLEXSPI_CFG_BLK_SIZE    (512)

/* FLEXSPI Feature related definitions */
#define FLEXSPI_FEATURE_HAS_PARALLEL_MODE 1

/* Lookup table related defintions */
#define CMD_INDEX_READ        0
#define CMD_INDEX_READSTATUS  1
#define CMD_INDEX_WRITEENABLE 2
#define CMD_INDEX_WRITE       4

#define CMD_LUT_SEQ_IDX_READ        0
#define CMD_LUT_SEQ_IDX_READSTATUS  1
#define CMD_LUT_SEQ_IDX_WRITEENABLE 3
#define CMD_LUT_SEQ_IDX_WRITE       9

#define CMD_SDR        0x01
#define CMD_DDR        0x21
#define RADDR_SDR      0x02
#define RADDR_DDR      0x22
#define CADDR_SDR      0x03
#define CADDR_DDR      0x23
#define MODE1_SDR      0x04
#define MODE1_DDR      0x24
#define MODE2_SDR      0x05
#define MODE2_DDR      0x25
#define MODE4_SDR      0x06
#define MODE4_DDR      0x26
#define MODE8_SDR      0x07
#define MODE8_DDR      0x27
#define WRITE_SDR      0x08
#define WRITE_DDR      0x28
#define READ_SDR       0x09
#define READ_DDR       0x29
#define LEARN_SDR      0x0A
#define LEARN_DDR      0x2A
#define DATSZ_SDR      0x0B
#define DATSZ_DDR      0x2B
#define DUMMY_SDR      0x0C
#define DUMMY_DDR      0x2C
#define DUMMY_RWDS_SDR 0x0D
#define DUMMY_RWDS_DDR 0x2D
#define JMP_ON_CS      0x1F
#define STOP           0

#define FLEXSPI_1PAD 0
#define FLEXSPI_2PAD 1
#define FLEXSPI_4PAD 2
#define FLEXSPI_8PAD 3

#define FLEXSPI_LUT_SEQ(cmd0, pad0, op0, cmd1, pad1, op1)      \
    (FLEXSPI_LUT_OPERAND0(op0) | FLEXSPI_LUT_NUM_PADS0(pad0) | \
     FLEXSPI_LUT_OPCODE0(cmd0) | FLEXSPI_LUT_OPERAND1(op1) |   \
     FLEXSPI_LUT_NUM_PADS1(pad1) | FLEXSPI_LUT_OPCODE1(cmd1))

#define NOR_CMD_INDEX_READ        CMD_INDEX_READ        //!< 0
#define NOR_CMD_INDEX_READSTATUS  CMD_INDEX_READSTATUS  //!< 1
#define NOR_CMD_INDEX_WRITEENABLE CMD_INDEX_WRITEENABLE //!< 2
#define NOR_CMD_INDEX_ERASESECTOR 3                     //!< 3
#define NOR_CMD_INDEX_PAGEPROGRAM CMD_INDEX_WRITE       //!< 4
#define NOR_CMD_INDEX_CHIPERASE   5                     //!< 5
#define NOR_CMD_INDEX_DUMMY       6                     //!< 6
#define NOR_CMD_INDEX_ERASEBLOCK  7                     //!< 7

#define NOR_CMD_LUT_SEQ_IDX_READ \
    CMD_LUT_SEQ_IDX_READ //!< 0  READ LUT sequence id in lookupTable stored in
                         //!< config block
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS \
    CMD_LUT_SEQ_IDX_READSTATUS //!< 1  Read Status LUT sequence id in
                               //!< lookupTable stored in config block
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_XPI \
    2 //!< 2  Read status DPI/QPI/OPI sequence id in lookupTable stored in
      //!< config block
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE \
    CMD_LUT_SEQ_IDX_WRITEENABLE //!< 3  Write Enable sequence id in lookupTable
                                //!< stored in config block
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_XPI \
    4 //!< 4  Write Enable DPI/QPI/OPI sequence id in lookupTable stored in
      //!< config block
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR \
    5 //!< 5  Erase Sector sequence id in lookupTable stored in config block
#define NOR_CMD_LUT_SEQ_IDX_ERASEBLOCK \
    8 //!< 8 Erase Block sequence id in lookupTable stored in config block
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM \
    CMD_LUT_SEQ_IDX_WRITE //!< 9  Program sequence id in lookupTable stored in
                          //!< config block
#define NOR_CMD_LUT_SEQ_IDX_CHIPERASE \
    11 //!< 11 Chip Erase sequence in lookupTable id stored in config block
#define NOR_CMD_LUT_SEQ_IDX_READ_SFDP \
    13 //!< 13 Read SFDP sequence in lookupTable id stored in config block
#define NOR_CMD_LUT_SEQ_IDX_RESTORE_NOCMD \
    14 //!< 14 Restore 0-4-4/0-8-8 mode sequence id in lookupTable stored in
       //!< config block
#define NOR_CMD_LUT_SEQ_IDX_EXIT_NOCMD \
    15 //!< 15 Exit 0-4-4/0-8-8 mode sequence id in lookupTable stored in config
       //!< blobkdefine NOR_CMD_INDEX_READ        CMD_INDEX_READ        //!< 0
#define NOR_CMD_INDEX_READSTATUS  CMD_INDEX_READSTATUS  //!< 1
#define NOR_CMD_INDEX_WRITEENABLE CMD_INDEX_WRITEENABLE //!< 2
#define NOR_CMD_INDEX_ERASESECTOR 3                     //!< 3
#define NOR_CMD_INDEX_PAGEPROGRAM CMD_INDEX_WRITE       //!< 4
#define NOR_CMD_INDEX_CHIPERASE   5                     //!< 5
#define NOR_CMD_INDEX_DUMMY       6                     //!< 6
#define NOR_CMD_INDEX_ERASEBLOCK  7                     //!< 7

#define NOR_CMD_LUT_SEQ_IDX_READ \
    CMD_LUT_SEQ_IDX_READ //!< 0  READ LUT sequence id in lookupTable stored in
                         //!< config block
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS \
    CMD_LUT_SEQ_IDX_READSTATUS //!< 1  Read Status LUT sequence id in
                               //!< lookupTable stored in config block
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_XPI \
    2 //!< 2  Read status DPI/QPI/OPI sequence id in lookupTable stored in
      //!< config block
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE \
    CMD_LUT_SEQ_IDX_WRITEENABLE //!< 3  Write Enable sequence id in lookupTable
                                //!< stored in config block
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_XPI \
    4 //!< 4  Write Enable DPI/QPI/OPI sequence id in lookupTable stored in
      //!< config block
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR \
    5 //!< 5  Erase Sector sequence id in lookupTable stored in config block
#define NOR_CMD_LUT_SEQ_IDX_ERASEBLOCK \
    8 //!< 8 Erase Block sequence id in lookupTable stored in config block
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM \
    CMD_LUT_SEQ_IDX_WRITE //!< 9  Program sequence id in lookupTable stored in
                          //!< config block
#define NOR_CMD_LUT_SEQ_IDX_CHIPERASE \
    11 //!< 11 Chip Erase sequence in lookupTable id stored in config block
#define NOR_CMD_LUT_SEQ_IDX_READ_SFDP \
    13 //!< 13 Read SFDP sequence in lookupTable id stored in config block
#define NOR_CMD_LUT_SEQ_IDX_RESTORE_NOCMD \
    14 //!< 14 Restore 0-4-4/0-8-8 mode sequence id in lookupTable stored in
       //!< config block
#define NOR_CMD_LUT_SEQ_IDX_EXIT_NOCMD \
    15 //!< 15 Exit 0-4-4/0-8-8 mode sequence id in lookupTable stored in config
       //!< blobk

#define IVT_MAJOR_VERSION       0x4
#define IVT_MAJOR_VERSION_SHIFT 0x4
#define IVT_MAJOR_VERSION_MASK  0xF
#define IVT_MINOR_VERSION       0x1
#define IVT_MINOR_VERSION_SHIFT 0x0
#define IVT_MINOR_VERSION_MASK  0xF

#define IVT_VERSION(major, minor)                                    \
    ((((major)&IVT_MAJOR_VERSION_MASK) << IVT_MAJOR_VERSION_SHIFT) | \
     (((minor)&IVT_MINOR_VERSION_MASK) << IVT_MINOR_VERSION_SHIFT))

/* IVT header */
#define IVT_TAG_HEADER 0xD1 /**< Image Vector Table */
#define IVT_SIZE       0x2000
#define IVT_PAR        IVT_VERSION(IVT_MAJOR_VERSION, IVT_MINOR_VERSION)
#define IVT_HEADER     (IVT_TAG_HEADER | (IVT_SIZE << 8) | (IVT_PAR << 24))

/* Set resume entry */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Reset_Handler[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)Reset_Handler)
#define BOOT_IMAGE_BASE     ((uint32_t)FLASH_BASE)
#define BOOT_IMAGE_SIZE     ((uint32_t)FLASH_SIZE)
#define BOOT_DATA_ADDRESS   &g_boot_data
#define IVT_ADDRESS         &image_vector_table
#define DCD_DATA_ADDRESS    dcd_data
#elif defined(__MCUXPRESSO)
extern uint32_t ResetISR[];
extern uint32_t __boot_hdr_start__[];
extern uint32_t __boot_hdr_ivt_loadaddr__[];
extern uint32_t __boot_hdr_boot_data_loadaddr__[];
extern uint32_t __boot_hdr_dcd_loadaddr__[];
extern uint32_t _boot_loadaddr[];
extern uint32_t _boot_size[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)ResetISR)
#define BOOT_IMAGE_BASE     ((uint32_t)_boot_loadaddr)
#define BOOT_IMAGE_SIZE     ((uint32_t)_boot_size)
#define BOOT_DATA_ADDRESS   ((uint32_t)__boot_hdr_boot_data_loadaddr__)
#define IVT_ADDRESS         ((uint32_t)__boot_hdr_ivt_loadaddr__)
#define DCD_DATA_ADDRESS    ((uint32_t)__boot_hdr_dcd_loadaddr__)
#elif defined(__ICCARM__)
extern uint32_t Reset_Handler[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)Reset_Handler)
#define BOOT_IMAGE_BASE     ((uint32_t)FLASH_BASE)
#define BOOT_IMAGE_SIZE     ((uint32_t)FLASH_SIZE)
#define BOOT_DATA_ADDRESS   &g_boot_data
#define IVT_ADDRESS         &image_vector_table
#define DCD_DATA_ADDRESS    dcd_data
#elif defined(__GNUC__)
extern uint32_t Reset_Handler[];
#define IMAGE_ENTRY_ADDRESS ((uint32_t)Reset_Handler)
#define BOOT_IMAGE_BASE     ((uint32_t)FLASH_BASE)
#define BOOT_IMAGE_SIZE     ((uint32_t)FLASH_SIZE)
#define BOOT_DATA_ADDRESS   &g_boot_data
#define IVT_ADDRESS         &image_vector_table
#define DCD_DATA_ADDRESS    dcd_data
#endif
#if defined(XIP_BOOT_HEADER_ENABLE) && (XIP_BOOT_HEADER_ENABLE == 1)
#if defined(XIP_BOOT_HEADER_DCD_ENABLE) && (1 == XIP_BOOT_HEADER_DCD_ENABLE)
#define DCD_ADDRESS DCD_DATA_ADDRESS
#else
#define DCD_ADDRESS 0
#endif
#endif
#define CSF_ADDRESS 0
#define IVT_RSVD    (uint32_t)(0x00000000)

#if __CORTEX_M == 7
#define FLASH_BASE FlexSPI1_AMBA_BASE
#elif __CORTEX_M == 4
#define FLASH_BASE FlexSPI1_ALIAS_BASE
#endif

#if defined(BOARD_FLASH_SIZE)
#define FLASH_SIZE BOARD_FLASH_SIZE
#else
#error "Please define macro BOARD_FLASH_SIZE"
#endif
#define PLUGIN_FLAG (uint32_t)0

//!@brief Definitions for FlexSPI Serial Clock Frequency
typedef enum _FlexSpiSerialClockFreq {
    kFlexSpiSerialClk_30MHz  = 1,
    kFlexSpiSerialClk_50MHz  = 2,
    kFlexSpiSerialClk_60MHz  = 3,
    kFlexSpiSerialClk_80MHz  = 4,
    kFlexSpiSerialClk_100MHz = 5,
    kFlexSpiSerialClk_120MHz = 6,
    kFlexSpiSerialClk_133MHz = 7,
    kFlexSpiSerialClk_166MHz = 8,
    kFlexSpiSerialClk_200MHz = 9,
} flexspi_serial_clk_freq_t;

//!@brief FlexSPI clock configuration type
enum {
    kFlexSpiClk_SDR, //!< Clock configure for SDR mode
    kFlexSpiClk_DDR, //!< Clock configurat for DDR mode
};

//!@brief FlexSPI Read Sample Clock Source definition
typedef enum _FlashReadSampleClkSource {
    kFlexSPIReadSampleClk_LoopbackInternally      = 0,
    kFlexSPIReadSampleClk_LoopbackFromDqsPad      = 1,
    kFlexSPIReadSampleClk_LoopbackFromSckPad      = 2,
    kFlexSPIReadSampleClk_ExternalInputFromDqsPad = 3,
} flexspi_read_sample_clk_t;

//!@brief Misc feature bit definitions
enum {
    kFlexSpiMiscOffset_DiffClkEnable = 0, //!< Bit for Differential clock enable
    kFlexSpiMiscOffset_Ck2Enable     = 1, //!< Bit for CK2 enable
    kFlexSpiMiscOffset_ParallelEnable = 2, //!< Bit for Parallel mode enable
    kFlexSpiMiscOffset_WordAddressableEnable =
        3, //!< Bit for Word Addressable enable
    kFlexSpiMiscOffset_SafeConfigFreqEnable =
        4, //!< Bit for Safe Configuration Frequency enable
    kFlexSpiMiscOffset_PadSettingOverrideEnable =
        5, //!< Bit for Pad setting override enable
    kFlexSpiMiscOffset_DdrModeEnable =
        6, //!< Bit for DDR clock confiuration indication.
};

//!@brief Flash Type Definition
enum {
    kFlexSpiDeviceType_SerialNOR  = 1, //!< Flash devices are Serial NOR
    kFlexSpiDeviceType_SerialNAND = 2, //!< Flash devices are Serial NAND
    kFlexSpiDeviceType_SerialRAM =
        3, //!< Flash devices are Serial RAM/HyperFLASH
    kFlexSpiDeviceType_MCP_NOR_NAND =
        0x12, //!< Flash device is MCP device, A1 is Serial NOR, A2 is Serial
              //!< NAND
    kFlexSpiDeviceType_MCP_NOR_RAM = 0x13, //!< Flash deivce is MCP device, A1
                                           //!< is Serial NOR, A2 is Serial RAMs
};

//!@brief Flash Pad Definitions
enum {
    kSerialFlash_1Pad  = 1,
    kSerialFlash_2Pads = 2,
    kSerialFlash_4Pads = 4,
    kSerialFlash_8Pads = 8,
};

//!@brief FlexSPI LUT Sequence structure
typedef struct _lut_sequence {
    uint8_t seqNum; //!< Sequence Number, valid number: 1-16
    uint8_t seqId;  //!< Sequence Index, valid number: 0-15
    uint16_t reserved;
} flexspi_lut_seq_t;

//!@brief Flash Configuration Command Type
enum {
    kDeviceConfigCmdType_Generic, //!< Generic command, for example: configure
                                  //!< dummy cycles, drive strength, etc
    kDeviceConfigCmdType_QuadEnable, //!< Quad Enable command
    kDeviceConfigCmdType_Spi2Xpi,    //!< Switch from SPI to DPI/QPI/OPI mode
    kDeviceConfigCmdType_Xpi2Spi,    //!< Switch from DPI/QPI/OPI to SPI mode
    kDeviceConfigCmdType_Spi2NoCmd,  //!< Switch to 0-4-4/0-8-8 mode
    kDeviceConfigCmdType_Reset,      //!< Reset device command
};

//!@brief FlexSPI Memory Configuration Block
typedef struct _FlexSPIConfig {
    uint32_t tag;     //!< [0x000-0x003] Tag, fixed value 0x42464346UL
    uint32_t version; //!< [0x004-0x007] Version,[31:24] -'V', [23:16] - Major,
                      //!< [15:8] - Minor, [7:0] - bugfix
    uint32_t reserved0;       //!< [0x008-0x00b] Reserved for future use
    uint8_t readSampleClkSrc; //!< [0x00c-0x00c] Read Sample Clock Source, valid
                              //!< value: 0/1/3
    uint8_t csHoldTime;       //!< [0x00d-0x00d] CS hold time, default value: 3
    uint8_t csSetupTime;      //!< [0x00e-0x00e] CS setup time, default value: 3
    uint8_t columnAddressWidth; //!< [0x00f-0x00f] Column Address with, for
                                //!< HyperBus protocol, it is fixed to 3, For
    //! Serial NAND, need to refer to datasheet
    uint8_t deviceModeCfgEnable; //!< [0x010-0x010] Device Mode Configure enable
                                 //!< flag, 1 - Enable, 0 - Disable
    uint8_t deviceModeType; //!< [0x011-0x011] Specify the configuration command
                            //!< type:Quad Enable, DPI/QPI/OPI switch,
    //! Generic configuration, etc.
    uint16_t
        waitTimeCfgCommands; //!< [0x012-0x013] Wait time for all configuration
                             //!< commands, unit: 100us, Used for
    //! DPI/QPI/OPI switch or reset command
    flexspi_lut_seq_t
        deviceModeSeq; //!< [0x014-0x017] Device mode sequence info, [7:0] - LUT
                       //!< sequence id, [15:8] - LUt
    //! sequence number, [31:16] Reserved
    uint32_t deviceModeArg;  //!< [0x018-0x01b] Argument/Parameter for device
                             //!< configuration
    uint8_t configCmdEnable; //!< [0x01c-0x01c] Configure command Enable Flag, 1
                             //!< - Enable, 0 - Disable
    uint8_t configModeType[3]; //!< [0x01d-0x01f] Configure Mode Type, similar
                               //!< as deviceModeTpe
    flexspi_lut_seq_t
        configCmdSeqs[3]; //!< [0x020-0x02b] Sequence info for Device
                          //!< Configuration command, similar as deviceModeSeq
    uint32_t reserved1;   //!< [0x02c-0x02f] Reserved for future use
    uint32_t configCmdArgs[3]; //!< [0x030-0x03b] Arguments/Parameters for
                               //!< device Configuration commands
    uint32_t reserved2;        //!< [0x03c-0x03f] Reserved for future use
    uint32_t
        controllerMiscOption; //!< [0x040-0x043] Controller Misc Options, see
                              //!< Misc feature bit definitions for more
    //! details
    uint8_t deviceType;    //!< [0x044-0x044] Device Type:  See Flash Type
                           //!< Definition for more details
    uint8_t sflashPadType; //!< [0x045-0x045] Serial Flash Pad Type: 1 - Single,
                           //!< 2 - Dual, 4 - Quad, 8 - Octal
    uint8_t serialClkFreq; //!< [0x046-0x046] Serial Flash Frequencey, device
                           //!< specific definitions, See System Boot
    //! Chapter for more details
    uint8_t lutCustomSeqEnable; //!< [0x047-0x047] LUT customization Enable, it
                                //!< is required if the program/erase cannot
    //! be done using 1 LUT sequence, currently, only applicable to HyperFLASH
    uint32_t reserved3[2]; //!< [0x048-0x04f] Reserved for future use
    uint32_t sflashA1Size; //!< [0x050-0x053] Size of Flash connected to A1
    uint32_t sflashA2Size; //!< [0x054-0x057] Size of Flash connected to A2
    uint32_t sflashB1Size; //!< [0x058-0x05b] Size of Flash connected to B1
    uint32_t sflashB2Size; //!< [0x05c-0x05f] Size of Flash connected to B2
    uint32_t
        csPadSettingOverride; //!< [0x060-0x063] CS pad setting override value
    uint32_t sclkPadSettingOverride; //!< [0x064-0x067] SCK pad setting override
                                     //!< value
    uint32_t dataPadSettingOverride; //!< [0x068-0x06b] data pad setting
                                     //!< override value
    uint32_t
        dqsPadSettingOverride; //!< [0x06c-0x06f] DQS pad setting override value
    uint32_t timeoutInMs; //!< [0x070-0x073] Timeout threshold for read status
                          //!< command
    uint32_t commandInterval;  //!< [0x074-0x077] CS deselect interval between
                               //!< two commands
    uint16_t dataValidTime[2]; //!< [0x078-0x07b] CLK edge to data valid time
                               //!< for PORT A and PORT B, in terms of 0.1ns
    uint16_t busyOffset;       //!< [0x07c-0x07d] Busy offset, valid value: 0-31
    uint16_t busyBitPolarity;  //!< [0x07e-0x07f] Busy flag polarity, 0 - busy
                               //!< flag is 1 when flash device is busy, 1 -
    //! busy flag is 0 when flash device is busy
    uint32_t lookupTable[64]; //!< [0x080-0x17f] Lookup table holds Flash
                              //!< command sequences
    flexspi_lut_seq_t
        lutCustomSeq[12];  //!< [0x180-0x1af] Customizable LUT Sequences
    uint32_t reserved4[4]; //!< [0x1b0-0x1bf] Reserved for future use
} flexspi_mem_config_t;

/*
 *  Serial NOR configuration block
 */
typedef struct _flexspi_nor_config {
    flexspi_mem_config_t
        memConfig;              // Common memory configuration info via FlexSPI
    uint32_t pageSize;          // Page size of Serial NOR
    uint32_t sectorSize;        // Sector size of Serial NOR
    uint8_t ipcmdSerialClkFreq; // Clock frequency for IP command
    uint8_t isUniformBlockSize; // Sector/Block size is the same
    uint8_t reserved0[2];       // Reserved for future use
    uint8_t serialNorType;      // Serial NOR Flash type: 0/1/2/3
    uint8_t
        needExitNoCmdMode; // Need to exit NoCmd mode before other IP command
    uint8_t halfClkForNonReadCmd; // Half the Serial Clock for non-read
                                  // command: true/false
    uint8_t needRestoreNoCmdMode; // Need to Restore NoCmd mode after IP
                                  // commmand execution
    uint32_t blockSize;           // Block size
    uint32_t reserve2[11];        // Reserved for future use
} flexspi_nor_config_t;

/*************************************
 *  IVT Data
 *************************************/
typedef struct _ivt_ {
    /** @ref hdr with tag #HAB_TAG_IVT, length and HAB version fields
     *  (see @ref data)
     */
    uint32_t hdr;
    /** Absolute address of the first instruction to execute from the
     *  image
     */
    uint32_t entry;
    /** Reserved in this version of HAB: should be NULL. */
    uint32_t reserved1;
    /** Absolute address of the image DCD: may be NULL. */
    uint32_t dcd;
    /** Absolute address of the Boot Data: may be NULL, but not interpreted
     *  any further by HAB
     */
    uint32_t boot_data;
    /** Absolute address of the IVT.*/
    uint32_t self;
    /** Absolute address of the image CSF.*/
    uint32_t csf;
    /** Reserved in this version of HAB: should be zero. */
    uint32_t reserved2;
} ivt;

/*************************************
 *  Boot Data
 *************************************/
typedef struct _boot_data_ {
    uint32_t start;  /* boot start location */
    uint32_t size;   /* size */
    uint32_t plugin; /* plugin flag - 1 if downloaded application is plugin */
    uint32_t placeholder; /* placehoder to make even 0x10 size */
} BOOT_DATA_T;

/* External Variables */
extern const BOOT_DATA_T g_boot_data;
#if defined(XIP_BOOT_HEADER_DCD_ENABLE) && (1 == XIP_BOOT_HEADER_DCD_ENABLE)
extern const uint8_t dcd_data[];
#endif

#endif /* __FLEXSPI_NOR_BOOT_H__ */
