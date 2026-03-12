#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include <sys/types.h>
#include <stddef.h>

/**
 * @brief Initializes the Virtual File System.
 */
void vfs_init(void);

/**
 * @brief Opens a device file and returns a file descriptor.
 *
 * @param path The path to the device to open.
 * @param flags Open flags (e.g., O_RDONLY). 
 * @param mode  File mode. Currently unused.
 * @return A non-negative file descriptor on success, or -1 on failure.
 */
int vfs_open(const char *path, int flags, int mode);

/**
 * @brief Reads data from an open file descriptor.
 *
 * @param fd The file descriptor.
 * @param buf The buffer to store the read data.
 * @param len The maximum number of bytes to read.
 * @return The number of bytes read on success, or -1 on failure.
 */
ssize_t vfs_read(int fd, void *buf, size_t len);

/**
 * @brief Writes data to an open file descriptor.
 *
 * @param fd The file descriptor.
 * @param buf The buffer containing the data to write.
 * @param len The number of bytes to write.
 * @return The number of bytes written on success, or -1 on failure.
 */
ssize_t vfs_write(int fd, const void *buf, size_t len);

/**
 * @brief Performs a device-specific control operation.
 *
 * @param fd The file descriptor.
 * @param cmd The ioctl command.
 * @param arg An optional argument for the command.
 * @return 0 on success, or -1 on failure.
 */
int vfs_ioctl(int fd, int cmd, void *arg);

/**
 * @brief Closes an open file descriptor.
 *
 * @param fd The file descriptor to close.
 * @return 0 on success, or -1 on failure.
 */
int vfs_close(int fd);

#endif /* KERNEL_VFS_H */