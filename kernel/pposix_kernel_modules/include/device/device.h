#ifndef KERNEL_DEVICE_H
#define KERNEL_DEVICE_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Defines the standard operations that a device driver must implement.
 */
typedef struct DevFileOps {
    ssize_t (*read)(void *priv, void *buf, size_t count);
    ssize_t (*write)(void *priv, const void *buf, size_t count);
    int     (*ioctl)(void *priv, int request, void *arg);
    int     (*open)(void *priv);
    int     (*close)(void *priv);
} DevFileOps;

/**
 * @brief Enumerates the different types of drivers.
 */
typedef enum {
    DRIVER_CHR = 0x0,
    DRIVER_NET,
    DRIVER_BLK,
    DRIVER_MNT,
} DriverType;

/**
 * @brief Represents a device instance registered with the kernel.
 */
typedef struct Device {
    const char      *path;  /**< The device's path in the VFS (e.g., "/dev/gpio2") */
    DriverType      type;   /**< The type of the driver */
    const DevFileOps *ops;  /**< A pointer to the driver's implemented operations */
    void            *priv;  /**< A pointer to the driver's private context data */
} Device;

/**
 * @brief Initializes the device manager subsystem.
 */
void device_manager_init(void);

/**
 * @brief Registers a new device with the device manager.
 *
 * @param dev A pointer to the device structure to register.
 * @return 0 on success, or a negative error code on failure.
 */
int device_register(Device *dev);

/**
 * @brief Finds a registered device by its path.
 *
 * @param path The path of the device to find.
 * @return A pointer to the device if found, otherwise NULL.
 */
Device *device_find(const char *path);

#endif /* KERNEL_DEVICE_H */
