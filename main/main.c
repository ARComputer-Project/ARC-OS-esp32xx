#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_err.h"
#include "esp32xx_gpio.h"

#include "device/device.h"
#include "vfs/vfs.h"
#include "driver/gpio.h"
#include "target.h"

static const char* TAG = "AppMain";

// ---- App Main ----
void app_main(void)
{
    // ---- Init kernel subsystems and driver ----
    device_manager_init();
    vfs_init();
    gpio_driver_global_init();
    target_init();

    ESP_LOGI(TAG, "Starting GPIO output test...");
    int led_fd = vfs_open("/dev/gpio4", 0, 0);

    if (led_fd < 0) {
        ESP_LOGE(TAG, "Failed to open LED device /dev/gpio4. Halting.");
        return;
    }

    ESP_LOGI(TAG, "Device /dev/gpio4 opened successfully with fd: %d", led_fd);
    
    vfs_ioctl(led_fd, GPIO_IOCTL_SET_DIR, (void *)1);

    ESP_LOGI(TAG, "Blinking LED...");
    uint8_t led_state = 0;
    while (1) {
        led_state = !led_state;
        vfs_write(led_fd, &led_state, 1);
        vTaskDelay(500);
    }

    vfs_close(led_fd);
    
}