SRC_FILES += \
	$(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
	$(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \

CFLAGS += -DFLOAT_ABI_SOFT
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DCONFIG_NFCT_PINS_AS_GPIOS
CFLAGS += -mfloat-abi=soft

ASMFLAGS += -mfloat-abi=soft
ASMFLAGS += -DFLOAT_ABI_SOFT
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA

SOFTDEVICE := S132
HEAP_SIZE := 2048
STACK_SIZE := 2048

LD_NAME := nrf52832_s132.ld

NRF_FAMILY := NRF52
