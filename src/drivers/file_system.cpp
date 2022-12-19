#include "file_system.h"

#include "fsl_gpio.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "fsl_sdmmc_common.h"
#include "fsl_sdmmc_host.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "fsl_iomuxc.h"
#pragma GCC diagnostic pop

#include "ff.h"

#include "logger.h"

#define SDMMC_SD_HOST_BASEADDR USDHC1
#define SDMMC_SD_HOST_IRQ      USDHC1_IRQn

#define SDMMC_SD_POWER_RESET_GPIO_BASE GPIO10
#define SDMMC_SD_POWER_RESET_GPIO_PIN  2U

/**
 * @brief Host interrupt priority.
 */
#define SDMMC_SD_HOST_IRQ_PRIORITY (5U)

/**
 * @brief DMA descriptor buffer size.
 */
#define SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE (32U)

/**
 * @brief Cache maintain function enabled for RW buffer.
 */
#define SDMMC_HOST_CACHE_CONTROL kSDMMCHOST_CacheControlRWBuffer

/**
 * @brief SD DMA buffer.
 */
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint32_t host_dma_buffer[SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE],
    SDMMCHOST_DMA_DESCRIPTOR_BUFFER_ALIGN_SIZE);

#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && \
    SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
/**
 * @brief Two cache line length for SD host driver maintain un-align transfer
 */
SDK_ALIGN(
    static uint8_t cache_line_align_buffer[SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U],
    SDMMC_DATA_BUFFER_ALIGN_SIZE);
#endif

/**
 * @brief Card detect structure. Used for defining callbacks for detecting when
 * a SD card is inserted.
 */
static sd_detect_card_t card_detect;

/**
 * @brief IO Voltage structure.
 */
static sd_io_voltage_t io_voltage = {.type = kSD_IOVoltageCtrlByHost,
                                     .func = NULL};

/**
 * @brief SDMMC host configuration structure.
 */
static sdmmchost_t host;

/**
 * @brief FATFS file system handle.
 */
static FATFS fat_file_system;

/**
 * @brief Handle to the current directory.
 */
static DIR current_directory;

/**
 * @brief Used for to localise the first entry in a directory with e.g. ls.
 */
static char current_directory_path[64] = "";

/**
 * @brief Volume to mount.
 */
const TCHAR volume[3U] = {SDDISK + '0', ':', '/'};

static void power_control(bool enable) {
    GPIO_PinWrite(SDMMC_SD_POWER_RESET_GPIO_BASE,
                  SDMMC_SD_POWER_RESET_GPIO_PIN,
                  enable ? 0 : 1);
}

static void data_pull_callback(uint32_t status) {
    if (status == kSD_DAT3PullDown) {
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 0xCU);

        // Power reset the card to clear DAT3 legacy status
        power_control(false);
        SDK_DelayAtLeastUs(1000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

        // Make sure the card is power on for DAT3 pull up
        power_control(true);
    } else {

        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 0x4U);
    }
}

namespace file_system {

    bool initialise() {

        // -- Pin configuration --

        CLOCK_EnableClock(kCLOCK_Iomuxc);

        IOMUXC_SetPinMux(IOMUXC_GPIO_AD_34_USDHC1_VSELECT, 0U);
        IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_00_USDHC1_CMD, 1U);
        IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_01_USDHC1_CLK, 1U);
        IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_02_USDHC1_DATA0, 1U);
        IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_03_USDHC1_DATA1, 1U);
        IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_04_USDHC1_DATA2, 1U);
        IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 1U);

        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_00_USDHC1_CMD, 0x04U);
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_01_USDHC1_CLK, 0x0CU);
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_02_USDHC1_DATA0, 0x04U);
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_03_USDHC1_DATA1, 0x04U);
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_04_USDHC1_DATA2, 0x04U);
        IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_05_USDHC1_DATA3, 0x04U);

        // --- DMA configuration ---

        host.dmaDesBuffer         = host_dma_buffer;
        host.dmaDesBufferWordsNum = SDMMC_HOST_DMA_DESCRIPTOR_BUFFER_SIZE;

#if ((defined __DCACHE_PRESENT) && __DCACHE_PRESENT) || \
    (defined FSL_FEATURE_HAS_L1CACHE && FSL_FEATURE_HAS_L1CACHE)
        host.enableCacheControl = SDMMC_HOST_CACHE_CONTROL;
#endif

#if defined SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER && \
    SDMMCHOST_ENABLE_CACHE_LINE_ALIGN_TRANSFER
        host.cacheAlignBuffer     = cache_line_align_buffer;
        host.cacheAlignBufferSize = SDMMC_DATA_BUFFER_ALIGN_SIZE * 2U;
#endif

        // --- Clock configuration ---

        // Configure clock for USDHC
        clock_root_config_t rootCfg = {};

        // SYS PLL2 528MHz.
        clock_sys_pll2_config_t sysPll2Config = {};
        sysPll2Config.ssEnable                = false,

        CLOCK_InitSysPll2(&sysPll2Config);
        CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd2, 24);

        rootCfg.mux = 4;
        rootCfg.div = 2;
        CLOCK_SetRootClock(kCLOCK_Root_Usdhc1, &rootCfg);

        // --- Card configuration ----

        g_sd.host                                = &host;
        g_sd.host->hostController.base           = SDMMC_SD_HOST_BASEADDR;
        g_sd.host->hostController.sourceClock_Hz = CLOCK_GetRootClockFreq(
            kCLOCK_Root_Usdhc1);

        g_sd.usrParam.cd         = &card_detect;
        g_sd.usrParam.pwr        = power_control;
        g_sd.usrParam.ioStrength = NULL;
        g_sd.usrParam.ioVoltage  = &io_voltage;
        g_sd.usrParam.maxFreq    = 200000000U;

        // -- Power reset pin configuration --

        gpio_pin_config_t power_reset_gpio_config = {
            .direction     = kGPIO_DigitalOutput,
            .outputLogic   = 0U,
            .interruptMode = kGPIO_NoIntmode};

        GPIO_PinInit(SDMMC_SD_POWER_RESET_GPIO_BASE,
                     SDMMC_SD_POWER_RESET_GPIO_PIN,
                     &power_reset_gpio_config);

        // -- Card detect configuration --

        card_detect.cdDebounce_ms = 100U;
        card_detect.type          = kSD_DetectCardByHostDATA3;
        // card_detect.callback      = cd;
        // card_detect.userData   = userData;
        card_detect.dat3PullFunc = data_pull_callback;

        power_control(true);

        NVIC_SetPriority(SDMMC_SD_HOST_IRQ, SDMMC_SD_HOST_IRQ_PRIORITY);

#if __CORTEX_M == 7
        // ERR050396
        // Errata description:
        // AXI to AHB conversion for CM7 AHBS port (port to access CM7 to TCM)
        // is by a NIC301 block, instead of XHB400 block. NIC301 doesn’t support
        // sparse write conversion. Any AXI to AHB conversion need XHB400, not
        // by NIC. This will result in data corruption in case of AXI sparse
        // write reaches the NIC301 ahead of AHBS. Errata workaround: For uSDHC,
        // don’t set the bit#1 of IOMUXC_GPR28 (AXI transaction is cacheable),
        // if write data to TCM aligned in 4 bytes; No such write access
        // limitation for OCRAM or external RAM
        IOMUXC_GPR->GPR28 &= (~IOMUXC_GPR_GPR28_AWCACHE_USDHC_MASK);
#endif

        if (SD_HostInit(&g_sd) != kStatus_Success) {
            logger::errorf("Failed to initialize SD host\r\n");
            return false;
        }

        if (!SD_IsCardPresent(&g_sd)) {
            logger::errorf("SD card is not inserted\r\n");
            return false;
        }

        // Cycle power for SD card
        SD_SetCardPower(&g_sd, false);
        SD_SetCardPower(&g_sd, true);

        // Mount the volume
        if (f_mount(&fat_file_system, volume, 0U)) {
            logger::errorf("Failed to mount SD card file system\r\n");
            return false;
        }

        // Change to the volume
        if (f_chdrive((char const*)&volume[0U])) {
            logger::errorf("Change to volume failed.\r\n");
            return false;
        }

        return true;
    }

    bool cd(const char* path) {

        if (strlen(path) > sizeof(current_directory_path) - 1) {
            logger::errorf(
                "Directory path is greater than maximum allowed size of %d\r\n",
                sizeof(current_directory_path) - 1);

            return false;
        }

        logger::debugf("Changing current directory to: %s\r\n", path);

        f_closedir(&current_directory);

        if (f_chdir(path) != FR_OK ||
            f_opendir(&current_directory, "./") != FR_OK) {
            logger::warnf("Failed to change current directory to: %s\r\n",
                          path);
            return false;
        }

        strcpy(current_directory_path, path);

        return true;
    }

    bool ls(char*** out_file_names, size_t* out_number_of_files) {

        *out_number_of_files = 0;

        FILINFO file_information;

        // First we need to find the first entry. If a series of ls commands are
        // executed without this, the directory pointer would point to the end
        // of the directory, so we need to update initially
        if (f_findfirst(&current_directory,
                        &file_information,
                        current_directory_path,
                        "*") != FR_OK) {
            return false;
        }

        size_t allocated_size = 16;

        *out_file_names = (char**)malloc(allocated_size * sizeof(char*));

        do {

            // Reached end of directory
            if (strcmp("", file_information.fname) == 0) {
                break;
            }

            const size_t file_name_length = strlen(file_information.fname);

            char* file_name = (char*)malloc((file_name_length + 1) *
                                            sizeof(char));

            strcpy(file_name, file_information.fname);

            // Append NULL termination
            file_name[file_name_length] = '\0';

            (*out_file_names)[(*out_number_of_files)++] = file_name;

            // Increase by reallocating with twice the size if the allocated
            // size is reached
            if (*out_number_of_files == allocated_size) {

                allocated_size *= 2;

                *out_file_names = (char**)realloc(*out_file_names,
                                                  allocated_size *
                                                      sizeof(char*));
            }
        } while (f_readdir(&current_directory, &file_information) == FR_OK);

        return true;
    }

    bool size(const char* path, uint32_t* out_file_size_ptr) {
        FIL file_handle;
        FRESULT status = f_open(&file_handle, path, FA_READ);

        if (status != FR_OK) {
            logger::errorf("Failed to open file for examining size: %s\r\n",
                           path);
            return false;
        }

        *out_file_size_ptr = f_size(&file_handle);

        f_close(&file_handle);

        return true;
    }

    bool
    read(const char* path, uint8_t* out_buffer, const uint32_t bytes_to_read) {

        logger::debugf("Reading %d bytes from: %s\r\n", bytes_to_read, path);

        FIL file_handle;

        FRESULT status = f_open(&file_handle, path, FA_READ);

        if (status != FR_OK) {
            logger::errorf("Failed to open file: %s\r\n", path);
            return false;
        }

        size_t bytes_read = 0;
        status = f_read(&file_handle, out_buffer, bytes_to_read, &bytes_read);

        if (status != FR_OK) {
            logger::errorf("Reading file failed with error code: %d\r\n",
                           status);
            f_close(&file_handle);
            return false;
        }

        if (bytes_read != bytes_to_read) {
            logger::errorf("Bytes read does not match the requested amount of "
                           "bytes to read\r\n");
            f_close(&file_handle);
            return false;
        }

        f_close(&file_handle);

        return true;
    }

    bool write(const char* path,
               const uint8_t* buffer,
               const uint32_t buffer_length) {

        logger::debugf("Writing %d bytes to: %s\r\n", buffer_length, path);

        FIL file_handle;

        FRESULT status =
            f_open(&file_handle, path, (FA_WRITE | FA_CREATE_ALWAYS));

        if (status != FR_OK && status != FR_EXIST) {
            logger::errorf(
                "Failed to open file for writing, error code: %d\r\n",
                status);
            return false;
        }

        uint32_t bytes_written = 0;

        status =
            f_write(&file_handle, buffer, buffer_length, (UINT*)&bytes_written);

        if (status != FR_OK) {
            logger::errorf("Failed to write file, error code: %d\r\n", status);
            f_close(&file_handle);
            return false;
        }

        if (bytes_written != buffer_length) {
            logger::errorf("Written bytes does not match the buffer length\r\n",
                           status);
            f_close(&file_handle);
            return false;
        }

        f_close(&file_handle);

        return true;
    }

    bool deinitialise() {

        if (g_sd.host == NULL) {
            return true;
        }

        if (f_unmount(volume)) {
            logger::errorf("Failed to unmount SD card file system\r\n");
            return false;
        }

        SD_Deinit(&g_sd);

        return true;
    }
}
