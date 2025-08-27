#include "esp32xx_gpio.h"
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>

// ESP-IDF includes
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"

// Maximum number of events in the per-pin ring buffer
#define GPIO_EVQ_CAP 32

#define GPIO_DRIVER_DEBUG
#ifdef GPIO_DRIVER_DEBUG
#include "esp_log.h"
#endif

#ifndef GPIO_DRIVER_DEBUG
#define ESP_LOGE //
#define ESP_LOGW //
#define ESP_LOGI //
#define ESP_LOGD //
#define ESP_LOGV //

#endif

//(TODO: move it to the .h)
/**
 * @brief Internal context structure for each GPIO device instance
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

/**
 * @brief Apply current pull-up/down setting to the hardware
 */
static void gpio_apply_pull(gpio_ctx_t *c)
{
    gpio_pulldown_dis(c->pin);
    gpio_pullup_dis(c->pin);
    int p = atomic_load(&c->pull);
    if (p == 1) gpio_pullup_en(c->pin);
    else if (p == 2) gpio_pulldown_en(c->pin);
}

/**
 * @brief ISR handler for GPIO interrupts. Pushes an event to the ring buffer.
 */
static void IRAM_ATTR gpio_isr_trampoline(void *arg)
{
    gpio_ctx_t *c = (gpio_ctx_t*)arg;
    if (!atomic_load(&c->irq_en)) return;

    uint32_t level = gpio_get_level(c->pin) & 1;
    uint64_t ts = esp_timer_get_time();

    unsigned h = atomic_load(&c->head);
    unsigned t = atomic_load(&c->tail);
    if ((h - t) < GPIO_EVQ_CAP) {
        c->evq[h % GPIO_EVQ_CAP] = (gpio_event_t){ .ts_us = ts, .level = (uint8_t)level };
        atomic_store(&c->head, h + 1);
    }
}

/**
 * @brief Called by VFS when device is opened
 */
static int gpio_open(void *priv)
{
    gpio_ctx_t *c = (gpio_ctx_t*)priv;
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << c->pin,
        .mode = atomic_load(&c->dir) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    gpio_apply_pull(c);
    gpio_isr_handler_add(c->pin, gpio_isr_trampoline, c); // Always attach ISR
    return 0;
}

/**
 * @brief Called by VFS when device is closed
 */
static int gpio_close(void *priv)
{
    gpio_ctx_t *c = (gpio_ctx_t*)priv;
    gpio_isr_handler_remove(c->pin);
    return 0;
}

/**
 * @brief Read from GPIO. Returns either:
 *        - A GPIO event list if count >= sizeof(gpio_event_t)
 *        - The current level as 1-byte if count < sizeof(gpio_event_t)
 */
static ssize_t gpio_read(void *priv, void *buf, size_t count)
{
    gpio_ctx_t *c = (gpio_ctx_t*)priv;

    if (count >= sizeof(gpio_event_t)) {
        size_t out = 0;
        while (out + sizeof(gpio_event_t) <= count) {
            unsigned t = atomic_load(&c->tail);
            unsigned h = atomic_load(&c->head);
            if (t == h) break; // No events
            ((gpio_event_t*)buf)[out / sizeof(gpio_event_t)] = c->evq[t % GPIO_EVQ_CAP];
            atomic_store(&c->tail, t + 1);
            out += sizeof(gpio_event_t);
        }
        if (out) return (ssize_t)out;
        // If no events available, fall back to level read below
    }

    uint8_t level = (uint8_t)(gpio_get_level(c->pin) & 1);
    if (count >= 1) {
        ((uint8_t*)buf)[0] = level;
        return 1;
    }
    return 0;
}

/**
 * @brief Write to GPIO (set level). Only works if direction is output.
 */
static ssize_t gpio_write(void *priv, const void *buf, size_t count)
{
    gpio_ctx_t *c = (gpio_ctx_t*)priv;
    ESP_LOGI("gpio driver", "ctx: %p dir: %d pin: %d", c, atomic_load(&c->dir), c->pin);

    if (!atomic_load(&c->dir)) return -1;
    
    if (count == 0) return 0;
    uint8_t level = ((const uint8_t*)buf)[0] ? 1 : 0;
    ESP_LOGI("gpio driver", "Setting level: %u", level);
    gpio_set_level(c->pin, level);
    return 10; //for test only // return 1;  
}

/**
 * @brief Handle ioctl commands for GPIO control
 */
static int gpio_ioctl(void *priv, int req, void *arg)
{
    gpio_ctx_t *c = (gpio_ctx_t*)priv;
    switch (req) {
    case GPIO_IOCTL_GET_INFO: {
        gpio_info_t *i = (gpio_info_t*)arg;
        i->pin = c->pin;
        i->bpp = 1;
        i->has_irq = 1;
        return 0;
    }
    case GPIO_IOCTL_SET_DIR: {
        int d = (int)(intptr_t)arg;
        atomic_store(&c->dir, d ? 1 : 0);
        gpio_set_direction(c->pin, d ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
        return 0;
    }
    case GPIO_IOCTL_SET_PULL: {
        int p = (int)(intptr_t)arg;
        atomic_store(&c->pull, p);
        gpio_apply_pull(c);
        return 0;
    }
    case GPIO_IOCTL_GET_LEVEL: {
        int *out = (int*)arg;
        *out = gpio_get_level(c->pin) & 1;
        return 0;
    }
    case GPIO_IOCTL_SET_IRQ_EDGE: {
        int e = (int)(intptr_t)arg;
        atomic_store(&c->irq_edge, e);
        gpio_int_type_t type = GPIO_INTR_DISABLE;
        if (e == 1) type = GPIO_INTR_POSEDGE;
        else if (e == 2) type = GPIO_INTR_NEGEDGE;
        else if (e == 3) type = GPIO_INTR_ANYEDGE;
        gpio_set_intr_type(c->pin, type);
        return 0;
    }
    case GPIO_IOCTL_ENABLE_IRQ:
        atomic_store(&c->irq_en, 1);
        gpio_intr_enable(c->pin);
        return 0;
    case GPIO_IOCTL_DISABLE_IRQ:
        gpio_intr_disable(c->pin);
        atomic_store(&c->irq_en, 0);
        return 0;
    case GPIO_IOCTL_CLEAR_QUEUE:
        atomic_store(&c->tail, atomic_load(&c->head));
        return 0;
    default:
        return -1;
    }
}

/**
 * @brief One-time global initialization of GPIO ISR service
 */
void gpio_driver_global_init(void)
{
    static bool inited = false;
    if (!inited) {
        gpio_install_isr_service(0);
        inited = true;
    }
}


/**
 * @brief VFS driver function table
 */
static const DevFileOps gpio_ops = {
    .read  = gpio_read,
    .write = gpio_write,
    .ioctl = gpio_ioctl,
    .open  = gpio_open,
    .close = gpio_close,
};

// ======= Code above should be in the kernel ====== 

// Static GPIO contexts for demo (GPIO2, GPIO35)
static gpio_ctx_t gpio0_ctx = { .pin = 2 };
static gpio_ctx_t gpio1_ctx = { .pin = 35 };

/**
 * @brief GPIO device table entries to be registered with the VFS
 */
DevFileEntry devtab_gpio[] = {
    { .path="/gpio2", .driver_type=DRIVER_CHR, .ops=&gpio_ops, .priv=&gpio0_ctx, .fixed_fd=0 },
    { .path="/gpio4", .driver_type=DRIVER_CHR, .ops=&gpio_ops, .priv=&gpio1_ctx, .fixed_fd=1 },
};

