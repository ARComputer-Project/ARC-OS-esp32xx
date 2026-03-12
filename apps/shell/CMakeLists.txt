idf_component_register(
    SRCS "src/commands.c" 
         "src/shell.c"
    REQUIRES console vfs driver fs_internal cardputer_lk
    INCLUDE_DIRS "include/"
)

