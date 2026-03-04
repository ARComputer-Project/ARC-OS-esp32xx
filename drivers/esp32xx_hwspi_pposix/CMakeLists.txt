idf_component_register(
    SRCS "src/esp32xx_hwspi.c"
    REQUIRES driver_structs driver esp_timer
    INCLUDE_DIRS "include/"
)
