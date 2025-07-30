/**
 * === ARC-OS SYSCALL TEST ===
 * This is a minimal test of syscall redirection using ESP-IDF's VFS.
 * It registers a fake device under `/dev/spi0` and reroutes libc `open()` and `write()`
 * through our own functions, simulating a syscall entry point into the ARC-OS driver layer.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "esp_vfs.h"
#include "esp_err.h"

// ---- mock write() implementation ----
static ssize_t my_write(int fd, const void *buf, size_t len) {
    const char *str = (const char *)buf;
    printf("[MOCK DRIVER] write(fd=%d, len=%d): \"%.*s\"\n", fd, (int)len, (int)len, str);
    return len;
}

static int my_open(const char *path, int flags, int mode) {
    printf("[MOCK DRIVER] open(path=%s, flags=%d)\n", path, flags);
    return 100;  // mock FD
}

static int my_close(int fd) {
    printf("[MOCK DRIVER] close(fd=%d)\n", fd);
    return 0;
}

// ---- VFS registration ----
void register_mock_driver() {
    esp_vfs_t vfs = {
        .open  = my_open,
        .write = my_write,
        .close = my_close,
    };

    ESP_ERROR_CHECK(esp_vfs_register("/dev", &vfs, NULL));
}

void app_main(void) {
    register_mock_driver();

    int fd = open("/dev/spi0", O_WRONLY);
    write(fd, "ping from userland", 19);
    close(fd);
}
