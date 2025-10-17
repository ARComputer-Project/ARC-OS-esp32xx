cmake_minimum_required(VERSION 3.5)

set(DRIVERS ./drivers/esp32xx_gpio
	    ./drivers/esp32xx_hwspi)

set(KERNEL_MODULES ./kernel/driver_structs)

set(TARGET_DIR ./targets/test_diy)

set(EXTRA_COMPONENT_DIRS ${DRIVERS}
			 ${KERNEL_MODULES}
			 ${TARGET_DIR})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ARC-OS-esp32xx)
