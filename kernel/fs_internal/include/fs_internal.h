#ifndef INTERNAL_FILESYSTEM_DRIVER
#define INTERNAL_FILESYSTEM_DRIVER

#define INTFS_MOUNT_PATH "/int"
#define INTFS_MAX_FILES  20

void int_fs_init(void);
void nvs_init(void);

#endif
