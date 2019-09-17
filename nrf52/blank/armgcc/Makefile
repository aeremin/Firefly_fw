PROJECT_NAME     := spi_pca10040
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

PROJ_DIR := ../../..

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := spi_gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(NRF_SDK_PATH)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(NRF_SDK_PATH)/components/libraries/log/src/nrf_log_backend_rtt.c \
  $(NRF_SDK_PATH)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(NRF_SDK_PATH)/components/libraries/log/src/nrf_log_backend_uart.c \
  $(NRF_SDK_PATH)/components/libraries/log/src/nrf_log_default_backends.c \
  $(NRF_SDK_PATH)/components/libraries/log/src/nrf_log_frontend.c \
  $(NRF_SDK_PATH)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(NRF_SDK_PATH)/components/boards/boards.c \
  $(NRF_SDK_PATH)/components/libraries/button/app_button.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_error.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_error_handler_gcc.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_error_weak.c \
  $(NRF_SDK_PATH)/components/libraries/scheduler/app_scheduler.c \
  $(NRF_SDK_PATH)/components/libraries/timer/app_timer_freertos.c \
  $(NRF_SDK_PATH)/components/libraries/util/app_util_platform.c \
  $(NRF_SDK_PATH)/components/libraries/util/nrf_assert.c \
  $(NRF_SDK_PATH)/components/libraries/atomic/nrf_atomic.c \
  $(NRF_SDK_PATH)/components/libraries/balloc/nrf_balloc.c \
  $(NRF_SDK_PATH)/external/fprintf/nrf_fprintf.c \
  $(NRF_SDK_PATH)/external/fprintf/nrf_fprintf_format.c \
  $(NRF_SDK_PATH)/components/libraries/memobj/nrf_memobj.c \
  $(NRF_SDK_PATH)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(NRF_SDK_PATH)/components/libraries/strerror/nrf_strerror.c \
  $(NRF_SDK_PATH)/integration/nrfx/legacy/nrf_drv_spi.c \
  $(NRF_SDK_PATH)/integration/nrfx/legacy/nrf_drv_uart.c \
  $(NRF_SDK_PATH)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(NRF_SDK_PATH)/modules/nrfx/soc/nrfx_atomic.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_spi.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_spim.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_uart.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/src/nrfx_uarte.c \
  $(NRF_SDK_PATH)/components/libraries/bsp/bsp.c \
  $(PROJ_DIR)/main.cpp \
  $(PROJ_DIR)/cc1101/cc1101.cpp \
  $(NRF_SDK_PATH)/external/segger_rtt/SEGGER_RTT.c \
  $(NRF_SDK_PATH)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
  $(NRF_SDK_PATH)/external/segger_rtt/SEGGER_RTT_printf.c \
  $(NRF_SDK_PATH)/modules/nrfx/mdk/system_nrf52.c \
  $(NRF_SDK_PATH)/external/freertos/source/croutine.c \
  $(NRF_SDK_PATH)/external/freertos/source/event_groups.c \
  $(NRF_SDK_PATH)/external/freertos/source/portable/MemMang/heap_1.c \
  $(NRF_SDK_PATH)/external/freertos/source/list.c \
  $(NRF_SDK_PATH)/external/freertos/portable/GCC/nrf52/port.c \
  $(NRF_SDK_PATH)/external/freertos/portable/CMSIS/nrf52/port_cmsis.c \
  $(NRF_SDK_PATH)/external/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c \
  $(NRF_SDK_PATH)/external/freertos/source/queue.c \
  $(NRF_SDK_PATH)/external/freertos/source/stream_buffer.c \
  $(NRF_SDK_PATH)/external/freertos/source/tasks.c \
  $(NRF_SDK_PATH)/external/freertos/source/timers.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(NRF_SDK_PATH)/components \
  $(NRF_SDK_PATH)/modules/nrfx/mdk \
  $(NRF_SDK_PATH)/components/libraries/scheduler \
  $(PROJ_DIR) \
  $(NRF_SDK_PATH)/components/libraries/timer \
  $(NRF_SDK_PATH)/components/libraries/strerror \
  $(NRF_SDK_PATH)/components/toolchain/cmsis/include \
  $(NRF_SDK_PATH)/components/libraries/util \
  ../config \
  $(NRF_SDK_PATH)/components/libraries/balloc \
  $(NRF_SDK_PATH)/components/libraries/ringbuf \
  $(NRF_SDK_PATH)/modules/nrfx/hal \
  $(NRF_SDK_PATH)/components/libraries/bsp \
  $(NRF_SDK_PATH)/components/libraries/log \
  $(NRF_SDK_PATH)/components/libraries/button \
  $(NRF_SDK_PATH)/modules/nrfx \
  $(NRF_SDK_PATH)/components/libraries/experimental_section_vars \
  $(NRF_SDK_PATH)/integration/nrfx/legacy \
  $(NRF_SDK_PATH)/components/libraries/delay \
  $(NRF_SDK_PATH)/integration/nrfx \
  $(NRF_SDK_PATH)/components/drivers_nrf/nrf_soc_nosd \
  $(NRF_SDK_PATH)/external/segger_rtt \
  $(NRF_SDK_PATH)/components/boards \
  $(NRF_SDK_PATH)/components/libraries/memobj \
  $(NRF_SDK_PATH)/modules/nrfx/drivers/include \
  $(NRF_SDK_PATH)/components/libraries/log/src \
  $(NRF_SDK_PATH)/external/fprintf \
  $(NRF_SDK_PATH)/components/libraries/atomic \
  $(NRF_SDK_PATH)/external/freertos/config \
  $(NRF_SDK_PATH)/external/freertos/portable/CMSIS/nrf52 \
  $(NRF_SDK_PATH)/external/freertos/portable/GCC/nrf52 \
  $(NRF_SDK_PATH)/external/freertos/source/include \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBOARD_PCA10040
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DFREERTOS
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags common to all targets
CXXFLAGS += $(OPT)
CXXFLAGS += -std=gnu++14

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74
ASMFLAGS += -DSWI_DISABLE0

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(NRF_SDK_PATH)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52832_xxaa: CFLAGS += -D__HEAP_SIZE=8192
nrf52832_xxaa: CFLAGS += -D__STACK_SIZE=8192
nrf52832_xxaa: ASMFLAGS += -D__HEAP_SIZE=8192
nrf52832_xxaa: ASMFLAGS += -D__STACK_SIZE=8192

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm


.PHONY: default help

# Default target - first one defined
default: nrf52832_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52832_xxaa
	@echo		sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

TEMPLATE_PATH := $(NRF_SDK_PATH)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash erase

# Flash the program
flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	nrfjprog -f nrf52 --program $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex --sectorerase
	nrfjprog -f nrf52 --reset

erase:
	nrfjprog -f nrf52 --eraseall

SDK_CONFIG_FILE := ../config/sdk_config.h
CMSIS_CONFIG_TOOL := $(NRF_SDK_PATH)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
