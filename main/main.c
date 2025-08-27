/**
 * EXAMPLE code for testing GPIO driver. 
 * NOT for prod, cornercases are not included.
 * 
 * 
 * TODO: fd assignement bug, open() doesn't return real fd
 * 
 * Custom fd range is not compatible with open()
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_err.h"
#include "esp32xx_gpio.h"

#include "driver/gpio.h"

// EXTERNAL
extern const DevFileEntry devtab_gpio[];
extern const DevFileOps gpio_ops;

#define DEV_COUNT 2

#define MY_VFS_MAX_FD 16
#define MY_VFS_BASE_FD 100 
static DevFileEntry *fd_table[MY_VFS_MAX_FD];

static const DevFileEntry *find_dev_by_fd(int fd) 
{
    int local_fd = fd - MY_VFS_BASE_FD;
    if (local_fd < 0 || local_fd >= MY_VFS_MAX_FD) return NULL;
    return fd_table[local_fd];
}


// ---- Local VFS ioctl wrapper ----
static int vfs_ioctl(int fd, int cmd, void *arg) 
{
    const DevFileEntry *dev = find_dev_by_fd(fd);
    if (dev && dev->ops && dev->ops->ioctl)
        return dev->ops->ioctl(dev->priv, cmd, arg);
    return -1;
}


// ---- Real C VFS wrappers (no flags processing) ---- (tried to implement custom fd table)
static int arc_open(const char *path, int flags, int mode) 
{
    ESP_LOGI("vfs", "OPEN called: %s", path);
    for (int i = 0; i < DEV_COUNT; i++) 
    {
        if (strcmp(path, devtab_gpio[i].path) == 0) 
        {
            for (int fd = 0; fd < MY_VFS_MAX_FD; fd++) 
            {
                if (fd_table[fd] == NULL)
                {
                    fd_table[fd] = (DevFileEntry *)&devtab_gpio[i]; // remove const-cast if possible
                    int final_fd = MY_VFS_BASE_FD + fd;
                    ESP_LOGI("MAIN", "Device found. path: %s → fd: %d", path, final_fd);
                    return fd;
                }
            }
            ESP_LOGE("vfs", "No available fd slot for: %s", path);
            return -1;
        } 
        else 
        {
            ESP_LOGW("vfs", "Skipping non-matching dev: %s", devtab_gpio[i].path);
        }
    }
    ESP_LOGE("vfs", "Device not found: %s", path);
    return -1;
}

static ssize_t arc_read(int fd, void *buf, size_t len) 
{
    const DevFileEntry *dev = find_dev_by_fd(fd);
    return dev->ops->read(devtab_gpio[fd].priv, buf, len);
}

static ssize_t arc_write(int fd, const void *buf, size_t len) 
{
    const DevFileEntry *dev = find_dev_by_fd(fd);
    return dev->ops->write(devtab_gpio[fd].priv, buf, len);
}

static int arc_close(int fd) 
{
    const DevFileEntry *dev = find_dev_by_fd(fd);
    return dev->ops->close(devtab_gpio[fd].priv);
}

// ---- Register custom VFS with ioctl passthrough ----
static void arc_vfs_register(void) 
{
    esp_vfs_t vfs = {
        .open  = arc_open,
        .read  = arc_read,
        .write = arc_write,
        .close = arc_close,
    };
    ESP_ERROR_CHECK(esp_vfs_register("/dev", &vfs, NULL));
    ESP_ERROR_CHECK(esp_vfs_register_fd_range(&vfs, NULL, MY_VFS_BASE_FD, MY_VFS_BASE_FD + MY_VFS_MAX_FD)); //(will not work with open())
}

// ---- App Main ----
void app_main(void)
{
    arc_vfs_register();
    gpio_driver_global_init();

    ESP_LOGI("MAIN", "open gpio2 + gpio4");
    int in_fd = open("/dev/gpio2", O_RDONLY);
    int out_fd = open("/dev/gpio4", O_WRONLY);

    ESP_LOGI("MAIN", "in_fd: %i; out_fd: %i", in_fd, out_fd);

    int ret = vfs_ioctl(in_fd, GPIO_IOCTL_SET_DIR, (void *)0);
    printf("ret ioctl in: %i\n", ret);
    ret = vfs_ioctl(out_fd, GPIO_IOCTL_SET_DIR, (void *)1);
    printf("ret ioctl out: %i\n", ret);
    
    //gpio_set_direction(35, GPIO_MODE_OUTPUT);
    ESP_LOGI("MAIN", "write and read");
    uint8_t val = 1;
    ret = arc_write(out_fd, &val, 1);
    printf("ret ioctl write: %i\n", ret);
    

    while (1)
    {
        val = val ^ 0x1;
        ret = write(out_fd, &val, 1);
        printf("ret ioctl write: %i\n", ret);
        //gpio_set_level(35, val);
        vTaskDelay(500);
    }


    gpio_event_t evs[8];
    int n = read(in_fd, evs, sizeof(evs));
    ESP_LOGI("MAIN", "read returned %d", n);
}
