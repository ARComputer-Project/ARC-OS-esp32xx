cmake_minimum_required(VERSION 3.5)

# ===================================
# ==Include driver directories here==
# ===================================
# Example: set(EXTRA_COMPONENT_DIRS ./drivers/<your_driver>)
set(EXTRA_COMPONENT_DIRS ./drivers/esp32xx_gpio
                         ./targets/test_diy)
# ===================================

# =========================================
# ==Include build target directories here==
# =========================================
# Example: set(EXTRA_COMPONENT_DIRS ./targets/<ESP32 SoC>/<target folder>)
#set(EXTRA_COMPONENT_DIRS ./targets/test_diy)
# =========================================

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ARC-OS-esp32xx)