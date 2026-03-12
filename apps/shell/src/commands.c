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

#include "shell.h"

int cmd_led_on(int argc, char** argv) 
{
    printf("LED ON\n");
    return 0;
}

int cmd_led_off(int argc, char** argv) 
{
    printf("LED OFF\n");
    return 0;
}

int cmd_reboot(int argc, char** argv) 
{
    printf("Rebooting...\n");
    esp_restart();
    return 0;
}


const esp_console_cmd_t shell_cmds[] = {
    {
        .command = "led_on",
        .help = "Turn LED on",
        .hint = NULL,
        .func = &cmd_led_on,
        .argtable = NULL
    },
    {
        .command = "led_off",
        .help = "Turn LED off",
        .hint = NULL,
        .func = &cmd_led_off,
        .argtable = NULL
    },
    {
        .command = "reboot",
        .help = "Reboot the system",
        .hint = NULL,
        .func = &cmd_reboot,
        .argtable = NULL
    }
};

const size_t shell_cmds_size =
    sizeof(shell_cmds) / sizeof(shell_cmds[0]);

