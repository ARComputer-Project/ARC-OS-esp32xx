cmake_minimum_required(VERSION 3.5)

# ===================================
# ==Include driver directories here==
# ===================================
# Example: set(EXTRA_COMPONENT_DIRS ./drivers/<your_driver>)
set(EXTRA_COMPONENT_DIRS ./kernel
                         ./drivers/esp32xx_gpio
                         ./targets/test_diy)
# ===================================

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ARC-OS-esp32xx)
