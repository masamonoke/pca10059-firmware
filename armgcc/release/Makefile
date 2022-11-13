PROJ_DIR := ../../src/release
DEPENDENCY_SRC = $(shell find ../$(PREF_DEP) -not -path '*/unity/*' -name '*.c')
CONFIG := ../../config

SRC_FILES +=  \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c


INC_FILES += \
	$(SDK_ROOT)/integration/nrfx/legacy/

include ../Makefile.common