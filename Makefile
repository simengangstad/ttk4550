CXX						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-g++
CC						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-gcc
LD						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-ld
AS						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-as
OC						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-objcopy
OD						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-objdump
OS						= $(MCUXPRESSO_IDE_BIN)arm-none-eabi-size

TARGET_CORE0			= core0_image
TARGET_CORE1			= core1_image

# ------------------- Locations ----------------------------
LD_SCRIPT_CORE0			= ./linker/mimxrt1160_cm7.ld
LD_SCRIPT_CORE1			= ./linker/mimxrt1160_cm4.ld

SDK_CMSIS_DIR			= lib/nxp/sdk/CMSIS
SDK_CMSIS_DSP_DIR		= lib/nxp/sdk/CMSIS/DSP
SDK_DEVICE_DIR			= lib/nxp/sdk/devices/MIMXRT1166
SDK_DRIVER_DIR			= $(SDK_DEVICE_DIR)/drivers
SDK_COMPONENTS_DIR		= lib/nxp/sdk/components
SDK_UTIL_DIR			= $(SDK_DEVICE_DIR)/utilities
SDK_MIDDLEWARE_DIR		= lib/nxp/sdk/middleware
SDK_LIB_DIR				= lib/nxp/sdk/lib

FATFS_DIR				= $(SDK_MIDDLEWARE_DIR)/fatfs/source
LODEPNG_DIR				= lib/lodepng


SRC_DIR					= src
BUILD_DIR				= build
BUILD_CORE0_DIR			= $(BUILD_DIR)/core0
BUILD_CORE1_DIR			= $(BUILD_DIR)/core1


# ------------------- Flash & Debug ------------------------
DEVICE_M7				= MIMXRT1166xxx5_M7
DEVICE_M4				= MIMXRT1166xxx5_M4
FLASH_SCRIPT			= scripts/jlink/flash.jlinkscript
DEBUG_SCRIPT			= scripts/jlink/debug.jlinkscript
SN						= 720725660

# --------------------- Flags ------------------------------

# Modified by the release/debug target
BUILD_TYPE_FLAGS		=

MCU_CORE0_SPEC			= cortex-m7
MCU_CORE1_SPEC			= cortex-m4

CPU						= -DCPU_MIMXRT1166DVM6A

CORE0					= -DCPU_MIMXRT1166DVM6A_cm7
CORE1					= -DCPU_MIMXRT1166DVM6A_cm4

SDK_FLAGS				= -DSDK_OS_BAREMETAL \
						  -DXIP_EXTERNAL_FLASH=1 \
						  -DXIP_BOOT_HEADER_ENABLE=1 \
						  -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
						  -D__USE_CMSIS \
						  -D__MCUXPRESSO_SDK \
						  -D__MCUXPRESSO \
						  -D__NEWLIB__ \
						  -D__MULTICORE_MASTER_SLAVE_M4SLAVE \
						  -D__USE_SHMEM


LODEPNG_FLAGS			= -DLODEPNG_NO_COMPILE_ENCODER \
						  -DLODEPNG_NO_COMPILE_DISK \
						  -DLODEPNG_NO_COMPILE_ACILLARY_CHUNKS \
						  -DLODEPNG_NO_COMPILE_CPP \
						  -DLODEPNG_NO_COMPILE_ERROR_TEXT

# Options
#
# -mcpu:					What cortex to use
# -gX:						Debug information level
# -c:						Don't invoke the linker
# -mfpu=fpv5-sp-16:			Floating-point hardware to use. Single precision
# -mfloat-abi=hard:			Uses FPU for floating point calculations
# -mthumb:					Uses thumb instruction set (T32), which is better for cache.
#							Reduces the size of the code with comperable performance.
# -fno-common:				Place uninitialized global variables in BSS section.
# -ffunction-sections:		Place each function into its own section in the output file
# -fdata-sections:			Place each data into its own section in the output file
# -ffreestanding:			Application is self hosted, where the main function is not
#							necessarily the starting point. For our case that is the
#							ResetISR
# -fno-builtin:				Prevent compiler from recognizing standard library functions
#							such as printf, malloc etc. These are defined in the application
#							code explicitely
# -fno-exceptions:			Disable use of exceptions within C++.
# -fmerge-constants:		Attempt to merge identical constants across compilation units
# -fstack-usage:			Generates a .su file which determines worst case stack usage
# -Wall:					Turn on all warnings
# -Wextra:					Turn all warnings into errors
# -Wpedantic:				Issue all warnings demanded by strict ISO C and ISO C++
# -lc:						Link with libc
# -lm:						Link with math library

FLAGS					= $(CPU) \
						  -mfpu=fpv5-sp-d16 \
						  -mfloat-abi=hard \
						  -mthumb \
						  $(SDK_FLAGS) \
						  $(LODEPNG_FLAGS) \
						  $(BUILD_TYPE_FLAGS) \
						  -c \
						  -Wall \
						  -Wextra \
						  -Wshadow \
						  -Wno-vla \
						  -fno-common \
						  -ffunction-sections \
						  -fdata-sections \
						  -ffreestanding \
						  -fno-builtin \
						  -fno-exceptions \
						  -fmerge-constants \
						  -fstack-usage

# -Werror \

CORE0_FLAGS				= -mcpu=$(MCU_CORE0_SPEC) \
						  $(CORE0) \
						  -DMULTICORE_MASTER

CORE1_FLAGS				= -mcpu=$(MCU_CORE1_SPEC) \
						  $(CORE1) \
						  -DMULTICORE_SLAVE

AS_FLAGS				= $(FLAGS) \
						  -std=gnu99 \
						  -x assembler-with-cpp

C_FLAGS					= $(FLAGS) \
						  -std=c17

CXX_FLAGS				= $(FLAGS) \
						  -std=c++20 \
						  -fno-rtti \
						  -Wno-register \
						  -Wpedantic \
						  -Wno-volatile

L_FLAGS					= -mfpu=fpv5-sp-d16 \
						  -mfloat-abi=hard \
						  -mthumb \
						  --specs=nano.specs \
						  --specs=nosys.specs \
						  $(CPU) \
						  $(SDK_FLAGS) \
						  $(LODEPNG_FLAGS) \
						  $(BUILD_TYPE_FLAGS) \
						  -nostdlib \
						  -Xlinker --gc-sections \
						  -Xlinker --print-memory-usage \
						  -Xlinker --sort-section=alignment \
						  -Xlinker --cref

L_CORE0_FLAGS			= $(CORE0_FLAGS) \
						  -T$(LD_SCRIPT_CORE0) \
						  -Xlinker -Map="$(BUILD_CORE0_DIR)/$(TARGET_CORE0).map" \
						  $(COMMON_INCLUDES) \
						  $(CM7_INCLUDES)

L_CORE1_FLAGS			= $(CORE1_FLAGS) \
						  -T$(LD_SCRIPT_CORE1) \
						  -Xlinker -Map="$(BUILD_CORE1_DIR)/$(TARGET_CORE1).map" \
						  $(COMMON_INCLUDES) \
						  $(CM4_INCLUDES)



# Silence warnings within the SDK
SDK_ERROR_FLAGS			= -Wno-array-bounds \
						  -Wno-unused-parameter \
						  -Wno-missing-field-initializers \
						  -Wno-maybe-uninitialized



CM7_INCLUDES			= -I$(SDK_DRIVER_DIR)/cm7 \
						  -Isrc/core/device/cm7

CM4_INCLUDES			= -I$(SDK_DRIVER_DIR)/cm4 \
						  -Isrc/core/device/cm4

COMMON_INCLUDES			= -I$(SDK_CMSIS_DIR)/Core/Include \
						  -I$(SDK_CMSIS_DSP_DIR)/Include \
						  -I$(SDK_CMSIS_DSP_DIR)/Include/dsp \
						  -I$(SDK_DEVICE_DIR) \
						  -I$(SDK_DRIVER_DIR) \
						  -I$(SDK_COMPONENTS_DIR)/osa \
						  -I$(SDK_COMPONENTS_DIR)/lists \
						  -I$(SDK_MIDDLEWARE_DIR)/fatfs/source \
						  -I$(SDK_MIDDLEWARE_DIR)/fatfs/source/fsl_sd_disk \
						  -I$(SDK_MIDDLEWARE_DIR)/sdmmc/sd \
						  -I$(SDK_MIDDLEWARE_DIR)/sdmmc/common \
						  -I$(SDK_MIDDLEWARE_DIR)/sdmmc/host/usdhc \
						  -I$(SDK_MIDDLEWARE_DIR)/sdmmc/osa \
						  -I$(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src \
						  -I$(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/include \
						  -I$(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/include/environment/bm \
						  -I$(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/include/platform/imxrt1160 \
						  -I$(LODEPNG_DIR) \
						  -Isrc \
						  -Isrc/core \
						  -Isrc/core/boot \
						  -Isrc/core/device \
						  -Isrc/drivers \
						  -Isrc/math \
						  -Isrc/util \
						  -Isrc/test \
						  -Isrc/vio


# -------------------- Sources & objects ------------------------------

PROJECT_CORE0_CPP_SRC	= $(wildcard src/core/*.cpp) \
						  $(filter-out src/main_cm4.cpp, $(wildcard src/*.cpp)) \
						  $(wildcard src/core/boot/*.cpp) \
						  $(wildcard src/core/device/cm7/*.cpp) \
						  $(wildcard src/drivers/*.cpp) \
						  $(wildcard src/util/*.cpp) \
						  $(wildcard src/vio/*.cpp) \
						  $(wildcard src/math/*.cpp) \
						  $(wildcard src/test/*.cpp) \

PROJECT_CORE0_C_SRC		= $(wildcard src/core/boot/*.c)

PROJECT_CORE0_AS_SRC	= src/core/device/cm7/inc_core1_bin.S

PROJECT_CORE1_CPP_SRC	= $(wildcard src/core/*.cpp) \
						  $(filter-out src/main_cm7.cpp, $(wildcard src/*.cpp)) \
						  $(wildcard src/core/boot/*.cpp) \
						  $(wildcard src/core/device/cm4/*.cpp) \
						  $(wildcard src/drivers/*.cpp) \
						  $(wildcard src/util/*.cpp) \
						  $(wildcard src/vio/*.cpp) \
						  $(wildcard src/math/*.cpp)

PROJECT_CORE1_C_SRC		=


SDK_DRIVER_SRC			= $(SDK_DRIVER_DIR)/fsl_anatop_ai.c \
						  $(SDK_DRIVER_DIR)/fsl_clock.c \
						  $(SDK_DRIVER_DIR)/fsl_common.c \
						  $(SDK_DRIVER_DIR)/fsl_common_arm.c \
						  $(SDK_DRIVER_DIR)/fsl_dcdc.c \
						  $(SDK_DRIVER_DIR)/fsl_gpio.c \
						  $(SDK_DRIVER_DIR)/fsl_lpuart.c \
						  $(SDK_DRIVER_DIR)/fsl_mu.c \
						  $(SDK_DRIVER_DIR)/fsl_pmu.c \
						  $(SDK_DRIVER_DIR)/fsl_usdhc.c

SDK_DRIVER_CORE0_SRC	= $(SDK_DRIVER_SRC) \
						  $(SDK_DRIVER_DIR)/cm7/fsl_cache.c

SDK_DRIVER_CORE1_SRC	= $(SDK_DRIVER_SRC) \
						  $(SDK_DRIVER_DIR)/cm4/fsl_cache.c


SDK_COMPONENTS_SRC		= $(SDK_COMPONENTS_DIR)/osa/fsl_os_abstraction_bm.c

SDK_UTIL_CORE0_SRC		=
SDK_UTIL_CORE1_SRC		=

SDK_MDLWR_CORE0_C_SRC	= $(wildcard $(SDK_MIDDLEWARE_DIR)/fatfs/source/*.c) \
						  $(SDK_MIDDLEWARE_DIR)/fatfs/source/fsl_sd_disk/fsl_sd_disk.c \
						  $(SDK_MIDDLEWARE_DIR)/sdmmc/sd/fsl_sd.c \
						  $(SDK_MIDDLEWARE_DIR)/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c \
						  $(SDK_MIDDLEWARE_DIR)/sdmmc/osa/fsl_sdmmc_osa.c \
						  $(SDK_MIDDLEWARE_DIR)/sdmmc/common/fsl_sdmmc_common.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src/mcmgr.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1160.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src/mcmgr_mu_internal.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_lite.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/rpmsg_lite/porting/environment/rpmsg_env_bm.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/rpmsg_lite/porting/platform/imxrt1160/rpmsg_platform.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/virtio/virtqueue.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/common/llist.c \

SDK_MDLWR_CORE0_CPP_SRC =

SDK_MDLWR_CORE1_C_SRC	= $(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src/mcmgr.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1160.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/mcmgr/src/mcmgr_mu_internal.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/rpmsg_lite/rpmsg_lite.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/rpmsg_lite/porting/environment/rpmsg_env_bm.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/rpmsg_lite/porting/platform/imxrt1160/rpmsg_platform.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/virtio/virtqueue.c \
						  $(SDK_MIDDLEWARE_DIR)/multicore/rpmsg_lite/lib/common/llist.c \

SDK_MDLWR_CORE1_CPP_SRC	=

SDK_DSP_CORE0_C_SRC		= $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_inverse_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_mult_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_trans_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_add_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_init_f32.c


SDK_DSP_CORE1_C_SRC		= $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_inverse_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_mult_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_trans_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_add_f32.c \
						  $(SDK_CMSIS_DSP_DIR)/Source/MatrixFunctions/arm_mat_init_f32.c



LODEPNG_SRC				= $(LODEPNG_DIR)/lodepng.cpp




PROJECT_CORE0_OBJS		= $(subst $(SRC_DIR), $(BUILD_CORE0_DIR), $(PROJECT_CORE0_CPP_SRC:.cpp=.o)) \
						  $(subst $(SRC_DIR), $(BUILD_CORE0_DIR), $(PROJECT_CORE0_C_SRC:.c=.o)) \
						  $(subst $(SRC_DIR), $(BUILD_CORE0_DIR), $(PROJECT_CORE0_AS_SRC:.S=.o))
PROJECT_CORE1_OBJS		= $(subst $(SRC_DIR), $(BUILD_CORE1_DIR), $(PROJECT_CORE1_CPP_SRC:.cpp=.o)) \
						  $(subst $(SRC_DIR), $(BUILD_CORE1_DIR), $(PROJECT_CORE1_C_SRC:.c=.o))


SDK_DRIVER_CORE0_OBJS	= $(subst $(SDK_DRIVER_DIR), $(BUILD_CORE0_DIR), $(SDK_DRIVER_CORE0_SRC:.c=.o))
SDK_DRIVER_CORE1_OBJS	= $(subst $(SDK_DRIVER_DIR), $(BUILD_CORE1_DIR), $(SDK_DRIVER_CORE1_SRC:.c=.o))

SDK_COMP_CORE0_OBJS		= $(subst $(SDK_COMPONENTS_DIR), $(BUILD_CORE0_DIR), $(SDK_COMPONENTS_SRC:.c=.o))
SDK_COMP_CORE1_OBJS		= $(subst $(SDK_COMPONENTS_DIR), $(BUILD_CORE1_DIR), $(SDK_COMPONENTS_SRC:.c=.o))

SDK_UTIL_CORE0_OBJS		= $(subst $(SDK_UTIL_DIR), $(BUILD_CORE0_DIR), $(SDK_UTIL_CORE0_SRC:.S=.o))
SDK_UTIL_CORE1_OBJS		= $(subst $(SDK_UTIL_DIR), $(BUILD_CORE1_DIR), $(SDK_UTIL_CORE1_SRC:.S=.o))

SDK_MDLWR_CORE0_OBJS	= $(subst $(SDK_MIDDLEWARE_DIR), $(BUILD_CORE0_DIR), $(SDK_MDLWR_CORE0_C_SRC:.c=.o)) \
						  $(subst $(SDK_MIDDLEWARE_DIR), $(BUILD_CORE0_DIR), $(SDK_MDLWR_CORE0_CPP_SRC:.cpp=.o))

SDK_MDLWR_CORE1_OBJS	= $(subst $(SDK_MIDDLEWARE_DIR), $(BUILD_CORE1_DIR), $(SDK_MDLWR_CORE1_C_SRC:.c=.o)) \
						  $(subst $(SDK_MIDDLEWARE_DIR), $(BUILD_CORE1_DIR), $(SDK_MDLWR_CORE1_CPP_SRC:.cpp=.o))

SDK_DSP_CORE0_C_OBJS	= $(subst $(SDK_CMSIS_DSP_DIR), $(BUILD_CORE0_DIR), $(SDK_DSP_CORE0_C_SRC:.c=.o))

SDK_DSP_CORE1_C_OBJS	= $(subst $(SDK_CMSIS_DSP_DIR), $(BUILD_CORE1_DIR), $(SDK_DSP_CORE1_C_SRC:.c=.o))



LODEPNG_OBJS			= $(subst $(LODEPNG_DIR), $(BUILD_CORE0_DIR), $(LODEPNG_SRC:.cpp=.o))

CORE0_OBJS				= $(PROJECT_CORE0_OBJS) \
						  $(SDK_DRIVER_CORE0_OBJS) \
						  $(SDK_COMP_CORE0_OBJS) \
						  $(SDK_UTIL_CORE0_OBJS) \
						  $(SDK_MDLWR_CORE0_OBJS) \
						  $(SDK_DSP_CORE0_C_SRC) \
						  $(LODEPNG_OBJS)

CORE1_OBJS				= $(PROJECT_CORE1_OBJS) \
						  $(SDK_DRIVER_CORE1_OBJS) \
						  $(SDK_COMP_CORE1_OBJS) \
						  $(SDK_UTIL_CORE1_OBJS) \
						  $(SDK_DSP_CORE1_C_SRC) \
						  $(SDK_MDLWR_CORE1_OBJS)


# ------------------------ Targets ----------------------------------
.PHONY: all release debug clean gdb flash

all: release

# CORE1 target is built first since we need its .bin file to include in CORE0 target
pre-build: $(BUILD_CORE1_DIR) $(BUILD_CORE0_DIR) | $(BUILD_CORE1_DIR)/$(TARGET_CORE1).bin

release: BUILD_TYPE_FLAGS += -DNDEBUG -O3 -Ofast -flto
release: pre-build
# Need to pass the build type flags here as this spawns another make
	@$(MAKE) --no-print-directory $(BUILD_CORE0_DIR)/$(TARGET_CORE0).hex BUILD_TYPE_FLAGS="-DNDEBUG -O3 -Ofast"


debug: BUILD_TYPE_FLAGS += -DDEBUG -g3 -O0 -DARM_MATH_MATRIX_CHECK
debug: pre-build
# Need to pass the build type flags here as this spawns another make
	@$(MAKE) --no-print-directory $(BUILD_CORE0_DIR)/$(TARGET_CORE0).hex BUILD_TYPE_FLAGS="-DDEBUG -g3 -O0"


$(BUILD_CORE0_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CORE0_FLAGS) $(CXX_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@

$(BUILD_CORE1_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CORE1_FLAGS) $(CXX_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@


$(BUILD_CORE0_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CORE0_FLAGS) $(C_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@

$(BUILD_CORE1_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CORE1_FLAGS) $(C_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@


$(BUILD_CORE0_DIR)/%.o: $(SRC_DIR)/%.S
	$(CC) $(CORE0_FLAGS) $(AS_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@



$(BUILD_CORE0_DIR)/%.o: $(SDK_DRIVER_DIR)/%.c
	$(CC) $(CORE0_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@

$(BUILD_CORE1_DIR)/%.o: $(SDK_DRIVER_DIR)/%.c
	$(CC) $(CORE1_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@


$(BUILD_CORE0_DIR)/%.o: $(SDK_COMPONENTS_DIR)/%.c
	$(CC) $(CORE0_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@

$(BUILD_CORE1_DIR)/%.o: $(SDK_COMPONENTS_DIR)/%.c
	$(CC) $(CORE1_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@


#$(BUILD_CORE0_DIR)/%.o: $(SDK_UTIL_DIR)/%.S
#	$(CC) $(CORE0_FLAGS) $(AS_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@
#
#$(BUILD_CORE1_DIR)/%.o: $(SDK_UTIL_DIR)/%.S
#	$(CC) $(CORE0_FLAGS) $(AS_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@
#

$(BUILD_CORE0_DIR)/%.o: $(SDK_MIDDLEWARE_DIR)/%.c
	$(CC) $(CORE0_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@

$(BUILD_CORE0_DIR)/%.o: $(SDK_MIDDLEWARE_DIR)/%.cpp
	$(CXX) $(CORE0_FLAGS) $(CXX_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@


$(BUILD_CORE1_DIR)/%.o: $(SDK_MIDDLEWARE_DIR)/%.c
	$(CC) $(CORE1_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@

$(BUILD_CORE1_DIR)/%.o: $(SDK_MIDDLEWARE_DIR)/%.cpp
	$(CXX) $(CORE1_FLAGS) $(CXX_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@


$(BUILD_CORE0_DIR)/%.o: $(SDK_CMSIS_DSP_DIR)/%.c
	$(CC) $(CORE0_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM7_INCLUDES) $< -o $@

$(BUILD_CORE1_DIR)/%.o: $(SDK_CMSIS_DSP_DIR)/%.c
	$(CC) $(CORE0_FLAGS) $(C_FLAGS) $(SDK_ERROR_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@




$(BUILD_CORE0_DIR)/%.o: $(LODEPNG_DIR)/%.cpp
	$(CXX) $(CORE0_FLAGS) $(CXX_FLAGS) $(COMMON_INCLUDES) $(CM4_INCLUDES) $< -o $@


$(BUILD_CORE0_DIR)/$(TARGET_CORE0).hex: $(CORE0_OBJS)
	$(CC) $^ $(L_CORE0_FLAGS) $(L_FLAGS) -o $(BUILD_CORE0_DIR)/$(TARGET_CORE0).elf
	$(OC) -O ihex $(BUILD_CORE0_DIR)/$(TARGET_CORE0).elf $@
	$(OS) $(BUILD_CORE0_DIR)/$(TARGET_CORE0).elf

$(BUILD_CORE1_DIR)/$(TARGET_CORE1).bin: $(CORE1_OBJS)
	$(CC) $^ $(L_CORE1_FLAGS) $(L_FLAGS) -o $(BUILD_CORE1_DIR)/$(TARGET_CORE1).elf
	$(OC) -O binary $(BUILD_CORE1_DIR)/$(TARGET_CORE1).elf $@
	$(OS) $(BUILD_CORE1_DIR)/$(TARGET_CORE1).elf

$(BUILD_CORE0_DIR):
	mkdir -p $(dir $(PROJECT_CORE0_OBJS))

	mkdir -p $(dir $(SDK_DRIVER_CORE0_OBJS))
	mkdir -p $(dir $(SDK_COMP_CORE0_OBJS))
	mkdir -p $(dir $(SDK_MDLWR_CORE0_OBJS))
	mkdir -p $(dir $(SDK_DSP_CORE0_C_OBJS))

	mkdir -p $(dir $(LODEPNG_OBJS))

$(BUILD_CORE1_DIR):
	mkdir -p $(dir $(PROJECT_CORE1_OBJS))

	mkdir -p $(dir $(SDK_DRIVER_CORE1_OBJS))
	mkdir -p $(dir $(SDK_COMP_CORE1_OBJS))
	mkdir -p $(dir $(SDK_MDLWR_CORE1_OBJS))



clean:
	rm -r $(BUILD_DIR)

flash: debug
	JLinkExe -nogui 1 \
			 -if SWD \
			 -speed 32000 \
			 -commanderscript $(FLASH_SCRIPT) \
			 -device $(DEVICE_M7) \
			 -SelectEmuBySn $(SN)

gdbcore0: debug
	JLinkGDBServer -nogui 1 \
				   -jlinkscriptfile $(DEBUG_SCRIPT) \
				   -device $(DEVICE_M7) \
				   -if SWD \
				   -speed 32000

gdbcore1: debug
	JLinkGDBServer -nogui 1 \
				   -jlinkscriptfile $(DEBUG_SCRIPT) \
				   -device $(DEVICE_M4) \
				   -if SWD \
				   -speed 32000


