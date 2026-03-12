cmake_minimum_required(VERSION 3.5)

option(LITE_KERNEL "Disable Pseudo-POSIX fetures" ON)

if(LITE_KERNEL)
    add_compile_definitions(LITE_KERNEL=1)

    set(APPS "./apps/shell")
    set(DRIVERS "")
    set(KERNEL_MODULES  "./kernel/fs_internal")
    # insert here your LiteKernel target
    set(TARGET_DIR ./targets/cardputer_lk)
else()
    add_compile_definitions(LITE_KERNEL=0)
    set(DRIVERS ./drivers/esp32xx_gpio_pposix
	            ./drivers/esp32xx_hwspi_pposix)

    set(KERNEL_MODULES ./kernel/)

    # insert here your pposix target
    set(TARGET_DIR ./targets/test_diy)
endif()


set(EXTRA_COMPONENT_DIRS ${APPS} ${DRIVERS} ${KERNEL_MODULES} ${TARGET_DIR})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ARC-OS-esp32xx)
