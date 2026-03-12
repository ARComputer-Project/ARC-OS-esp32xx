#include "device/device.h"
#include <string.h>
#include "esp_log.h"

#define MAX_DEVICES 16
static const char* TAG = "DeviceManager";

static Device* device_table[MAX_DEVICES];
static int device_count = 0;

/**
 * @brief Initializes the device manager.
 */
void device_manager_init(void) {
    memset(device_table, 0, sizeof(device_table));
    device_count = 0;
    ESP_LOGI(TAG, "Initialized");
}

/**
 * @brief Registers a new device with the system.
 */
int device_register(Device *dev) {
    if (device_count >= MAX_DEVICES) {
        ESP_LOGE(TAG, "Cannot register device '%s', device table is full", dev->path);
        return -1;
    }
    device_table[device_count++] = dev;
    ESP_LOGI(TAG, "Registered device: %s", dev->path);
    return 0;
}

/**
 * @brief Finds a device by its file path.
 */
Device *device_find(const char *path) {
    for (int i = 0; i < device_count; i++) {
        if (device_table[i] && strcmp(path, device_table[i]->path) == 0) {
            return device_table[i];
        }
    }
    
    return NULL;
}