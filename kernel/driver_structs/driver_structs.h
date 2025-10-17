#ifndef KERNEL_DRIVER_STRUCTS
#define KERNEL_DRIVER_STRUCTS

#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>

typedef uint8_t driver_type_t;

enum 
{
    DRIVER_CHR = 0x0,
    DRIVER_NET,
    DRIVER_BLK,
    DRIVER_MNT,
};

// --- char/file device driver (defult) ---

typedef struct DevFileOps{
    ssize_t (*read)(void *priv, void *buf, size_t count);
    ssize_t (*write)(void *priv, const void *buf, size_t count);
    int     (*ioctl)(void *priv, int request, void *arg);
    int     (*open)(void *priv);
    int     (*close)(void *priv);
} DevFileOps;
// ----------------------------------------

typedef struct DevFileEntry 
{
    const char * path;
    driver_type_t driver_type;
    const DevFileOps * ops;
    void * priv;
    int16_t fixed_fd;
} DevFileEntry;

#endif
