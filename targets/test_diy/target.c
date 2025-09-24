#include "target.h"



gpio_ctx_t gpio0_ctx = { .pin = 2 };
gpio_ctx_t gpio1_ctx = { .pin = 35 };

/**
 * @brief GPIO device table entries to be registered with the VFS
 */
DevFileEntry devtab[] = {
    { .path="/gpio2", .driver_type=DRIVER_CHR, .ops=&gpio_ops, .priv=&gpio0_ctx, .fixed_fd=0 },
    { .path="/gpio4", .driver_type=DRIVER_CHR, .ops=&gpio_ops, .priv=&gpio1_ctx, .fixed_fd=1 },
};