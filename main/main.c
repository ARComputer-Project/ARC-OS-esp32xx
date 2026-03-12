#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "shell.h"
#include "fs_internal.h"
#include "target.h"

// size is in WORDS (not in Bytes)
#define DRIVER_STACK_SIZE_BASIC     2048 * 1
#define APP_STACK_SIZE_BIG          2048 * 2
#define APP_STACK_SIZE_BASIC        2048 * 1

#define CORE1 1
#define CORE0 0


static const char* TAG = "AppMain";




// ---- App Main ----
void app_main(void)
{ 
    static StackType_t serial_tty_stack[APP_STACK_SIZE_BIG];

    static StaticTask_t serial_tty_buf;
    ESP_LOGI(TAG, "Initializing essential kernel modules!");
    nvs_init();
    int_fs_init();

    ESP_LOGI(TAG, "Creating system tasks!");
    
#ifdef SHELL_ENABLE
    xTaskCreateStatic(shell_task, "Serial Shell", 
                      APP_STACK_SIZE_BIG, NULL,
                      tskIDLE_PRIORITY + 1,
                      serial_tty_stack, &serial_tty_buf);
#endif
}
