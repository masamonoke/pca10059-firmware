BUILD_TYPE ?= release
PLATFORM ?= none

dfu:
	cd armgcc/$(BUILD_TYPE) && make dfu

dfu_package:
	cd armgcc/$(BUILD_TYPE) && make dfu_package

clean:
	cd armgcc/$(BUILD_TYPE) && make clean
