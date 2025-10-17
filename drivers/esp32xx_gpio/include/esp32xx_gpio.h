#ifndef ESP32XX_DRIVER_GPIO_H
#define ESP32XX_DRIVER_GPIO_H
 
#include <stdatomic.h>

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"

#include "driver_structs.h"


// Maximum number of events in the per-pin ring buffer
#define GPIO_EVQ_CAP 32


/* Compatibility with C++ code */
#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Context structure for each GPIO device instance
 */
typedef struct {
    int pin;
    atomic_int dir;        // 0 = input, 1 = output
    atomic_int pull;       // 0 = none, 1 = pull-up, 2 = pull-down
    atomic_int irq_edge;   // 0 = none, 1 = rising, 2 = falling, 3 = both
    atomic_int irq_en;     // 0 = disabled, 1 = enabled

    gpio_event_t evq[GPIO_EVQ_CAP]; // ring buffer for GPIO edge events
    atomic_uint  head;              // ISR producer index
    atomic_uint  tail;              // read consumer index
} gpio_ctx_t;



extern const DevFileOps gpio_ops;

/**
 *  Driver init function
 */

extern void gpio_driver_global_init(void);

#ifdef __cplusplus
}
#endif

#endif
