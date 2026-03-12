idf_component_register(
    SRCS "src/fs_internal.c"
    REQUIRES driver vfs nvs_flash fatfs cardputer_lk
    INCLUDE_DIRS "include/"
)
