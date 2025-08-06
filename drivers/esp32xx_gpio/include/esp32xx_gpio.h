#ifndef ESP32_DRIVER_GPIO_H
#define ESP32_DRIVER_GPIO_H

#include <stdint.h>
#include <sys/types.h>  
#include <stddef.h>     


/* Compatibility with C++ code */
#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t driver_type_t;

enum 
{
    DRIVER_CHR = 0x0,
    DRIVER_NET,
    DRIVER_BLK,
    DRIVER_MNT,
};


typedef struct DevFileOps{
    ssize_t (*read)(void *priv, void *buf, size_t count);
    ssize_t (*write)(void *priv, const void *buf, size_t count);
    int     (*ioctl)(void *priv, int request, void *arg);
    int     (*open)(void *priv);
    int     (*close)(void *priv);
} DevFileOps;

typedef struct DevFileEntry 
{
    const char * path;
    driver_type_t driver_type;
    const DevFileOps * ops;
    void * priv;
    int16_t fixed_fd;
} DevFileEntry;


// ---- GPIO API (ioctl) ----
enum {
    GPIO_IOCTL_GET_INFO = 0x100,
    GPIO_IOCTL_SET_DIR,       // arg: int (0=in, 1=out)
    GPIO_IOCTL_SET_PULL,      // arg: int (0=off,1=up,2=down)
    GPIO_IOCTL_GET_LEVEL,     // arg: int* out
    GPIO_IOCTL_SET_IRQ_EDGE,  // arg: int (0=none,1=rising,2=falling,3=both)
    GPIO_IOCTL_ENABLE_IRQ,    // no arg
    GPIO_IOCTL_DISABLE_IRQ,   // no arg
    GPIO_IOCTL_CLEAR_QUEUE,   // no arg
};

typedef struct {
    uint8_t  pin;     // ESP GPIO num
    uint8_t  bpp;     // 1
    uint8_t  has_irq; // 0/1
} gpio_info_t;

typedef struct {
    uint64_t ts_us;
    uint8_t  level; // 0/1
} gpio_event_t;

/**
 *  Kernel init functions
 */

extern void gpio_driver_global_init(void);

#ifdef __cplusplus
}
#endif

#endif