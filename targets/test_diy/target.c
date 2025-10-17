#include "target.h"
#include "device/device.h"
#include "esp32xx_gpio.h"


static gpio_ctx_t gpio_pin2_ctx = { .pin = 2 };
static gpio_ctx_t gpio_pin35_ctx = { .pin = 35 };

static Device gpio_dev_2 = {
    .path = "/dev/gpio2",
    .type = DRIVER_CHR,
    .ops = &gpio_ops,       
    .priv = &gpio_pin2_ctx,
};

static Device gpio_dev_35 = {
    .path = "/dev/gpio4",
    .type = DRIVER_CHR,
    .ops = &gpio_ops,       
    .priv = &gpio_pin35_ctx,
};

/**
 * @brief Initializes and registers the devices for this target.
 */
void target_init(void) {
    device_register(&gpio_dev_2);
    device_register(&gpio_dev_35);
}