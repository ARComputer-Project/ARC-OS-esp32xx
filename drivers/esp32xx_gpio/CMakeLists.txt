idf_component_register(
    SRCS "src/esp32xx_gpio.c"
    REQUIRES kernel driver esp_timer
    INCLUDE_DIRS "include/"
)