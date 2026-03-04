#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_cdcacm.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "target.h"

// size is in WORDS (not in Bytes)
#define DRIVER_STACK_SIZE_BASIC    2048 * 1
#define APP_STACK_SIZE_BASIC    2048 * 1

#define CORE1 1
#define CORE0 0


static const char* TAG = "AppMain";




void stty_show_intro()
{
    printf("Welcome to the ARC OS Shell v0.01!\n");
    printf("Type \"help\" or \"?\" to see the available commands");
}

// Example command handlers
static int cmd_help(int argc, char** argv) {
    printf("Available commands: help, led_on, led_off, reboot\n");
    return 0;
}

static int cmd_led_on(int argc, char** argv) {
    printf("LED ON\n");
    return 0;
}

static int cmd_led_off(int argc, char** argv) {
    printf("LED OFF\n");
    return 0;
}

static int cmd_reboot(int argc, char** argv) {
    printf("Rebooting...\n");
    esp_restart();
    return 0;
}

// Shell task using ESP-IDF console
void console(void* params)
{
    // --- Initialize console ---
    fflush(stdout);
    fsync(fileno(stdout));
    esp_vfs_dev_cdcacm_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_cdcacm_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    setvbuf(stdin, NULL, _IONBF, 0);


    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 128,
    };
    esp_console_init(&console_config);

    // --- Register commands ---
    esp_console_cmd_t cmd = {
        .command = "help",
        .help = "Show available commands",
        .func = &cmd_help,
    };
    esp_console_cmd_register(&cmd);

    cmd.command = "led_on";
    cmd.help = "Turn LED on";
    cmd.func = &cmd_led_on;
    esp_console_cmd_register(&cmd);

    cmd.command = "led_off";
    cmd.help = "Turn LED off";
    cmd.func = &cmd_led_off;
    esp_console_cmd_register(&cmd);

    cmd.command = "reboot";
    cmd.help = "Reboot the system";
    cmd.func = &cmd_reboot;
    esp_console_cmd_register(&cmd);

    // --- Show intro ---
    stty_show_intro();

    // --- Main loop ---
    while (1)
    {
        char* line = linenoise("> "); // Read line with editing & history
        if (line != NULL)
        {
            // Execute command
            int ret;
            esp_err_t err = esp_console_run(line, &ret);
            if (err == ESP_ERR_NOT_FOUND) {
                printf("Unrecognized command\n");
            } else if (err == ESP_ERR_INVALID_ARG) {
                // command was empty
            } else if (err == ESP_OK && ret != ESP_OK) {
                printf("Command returned non-zero error code: 0x%x (%s)\n", 
                        ret, esp_err_to_name(ret));
            } else if (err != ESP_OK) {
                printf("Internal error: %s\n", esp_err_to_name(err));
            }
                linenoiseFree(line);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}



// ---- App Main ----
void app_main(void)
{ 
    static StackType_t serial_tty_stack[APP_STACK_SIZE_BASIC];

    static StaticTask_t serial_tty_buf;

    xTaskCreateStatic(console, "Serial Shell", 
                      APP_STACK_SIZE_BASIC, NULL,
                      tskIDLE_PRIORITY + 1,
                      serial_tty_stack, &serial_tty_buf);
}
