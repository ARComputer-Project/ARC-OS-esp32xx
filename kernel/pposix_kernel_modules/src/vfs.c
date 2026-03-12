#include "vfs/vfs.h"
#include "device/device.h"
#include "esp_log.h"
#include <errno.h>
#include <string.h>

#define VFS_MAX_FD 16 
static const char* TAG = "VFS";

typedef struct {
    Device *dev;    // A pointer to the underlying device
    bool in_use;    // A flag to check if this FD slot is occupied
} FileDescriptor;

static FileDescriptor fd_table[VFS_MAX_FD];

void vfs_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
    ESP_LOGI(TAG, "Initialized");
}

int vfs_open(const char *path, int flags, int mode) {
    Device *dev = device_find(path);
    if (!dev) {
        ESP_LOGE(TAG, "Device not found: %s", path);
        errno = ENOENT;
        return -1;
    }

    for (int fd = 0; fd < VFS_MAX_FD; fd++) {
        if (!fd_table[fd].in_use) {
            fd_table[fd].in_use = true;
            fd_table[fd].dev = dev;

            if (dev->ops && dev->ops->open) {
                if (dev->ops->open(dev->priv) < 0) {
                    fd_table[fd].in_use = false; 
                    return -1;
                }
            }
            ESP_LOGI(TAG, "Opened '%s', assigned fd: %d", path, fd);
            return fd; 
        }
    }

    ESP_LOGE(TAG, "No available file descriptors for '%s'", path);
    errno = ENFILE;
    return -1;
}

static Device* get_device_from_fd(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FD || !fd_table[fd].in_use) {
        errno = EBADF; 
        return NULL;
    }
    return fd_table[fd].dev;
}

ssize_t vfs_read(int fd, void *buf, size_t len) {
    Device *dev = get_device_from_fd(fd);
    if (!dev || !dev->ops || !dev->ops->read) {
        return -1;
    }
    return dev->ops->read(dev->priv, buf, len);
}

ssize_t vfs_write(int fd, const void *buf, size_t len) {
    Device *dev = get_device_from_fd(fd);
    if (!dev || !dev->ops || !dev->ops->write) {
        return -1;
    }
    return dev->ops->write(dev->priv, buf, len);
}

int vfs_ioctl(int fd, int cmd, void *arg) {
    Device *dev = get_device_from_fd(fd);
    if (!dev || !dev->ops || !dev->ops->ioctl) {
        return -1;
    }
    return dev->ops->ioctl(dev->priv, cmd, arg);
}

int vfs_close(int fd) {
    Device *dev = get_device_from_fd(fd);
    if (!dev) {
        return -1;
    }

    int ret = 0;
    if (dev->ops && dev->ops->close) {
        ret = dev->ops->close(dev->priv);
    }

    fd_table[fd].in_use = false;
    fd_table[fd].dev = NULL;

    ESP_LOGI(TAG, "Closed fd: %d", fd);
    return ret;
}