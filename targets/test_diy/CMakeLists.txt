idf_component_register(
    SRCS "target.c"
    REQUIRES esp32xx_gpio driver
    INCLUDE_DIRS "."
)