PROJ_DIR := ../../src/release
DEPENDENCY_SRC = $(shell find ../$(PREF_DEP) -not -path '*/unity/*' -name '*.c')
CONFIG := ../../config

SRC_FILES +=  \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c \
	$(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c \
	$(SDK_ROOT)/modules/nrfx/hal/nrf_nvmc.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_nvmc.c \


INC_FILES += \
	$(SDK_ROOT)/integration/nrfx/legacy \
	$(SDK_ROOT)/modules/nrfx/drivers/include \

include ../Makefile.common

#to reset this value first make clean
ESTC_USB_CLI_ENABLED ?= 1
CFLAGS += -DESTC_USB_CLI_ENABLED=$(ESTC_USB_CLI_ENABLED)