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
#include "esp_log.h"

#include "freertos/FreeRTOS.h"

#include "shell.h"
#include "target.h"

#define TAG "SHELL_APP"

static void shell_show_intro()
{
    printf("Welcome to the ARC OS Shell v0.01!\n");
    printf("Type \"help\" or \"?\" to see the available commands\n");
}

static void shell_register(const esp_console_cmd_t * cmds, size_t size)
{
    for (size_t i = 0; i < size; i++) 
    {
        esp_console_cmd_register(&cmds[i]);
    }
}

static void shell_register_help()
{
    esp_console_register_help_command();
}

static void shell_init()
{
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
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

    /* Set command maximum length */
    linenoiseSetMaxLineLen(console_config.max_cmdline_length);

    /* Don't return empty lines */
    linenoiseAllowEmpty(false);

#ifdef SHELL_HISTORY_ENABLE
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
    const int probe_status = linenoiseProbe();
    if (probe_status)
    {
        linenoiseSetDumbMode(1);
    }

    shell_register(shell_cmds, shell_cmds_size);
    shell_register_help();
}

// Shell task using ESP-IDF console
void shell_task(void* params)
{
    // --- Initialize console ---
    shell_init();
    // --- Show intro ---
    shell_show_intro();
    
    
    if (linenoiseIsDumbMode()) 
    {
        ESP_LOGW(TAG, "Your terminal application does not support escape sequences.\nLine editing and history features are disabled.\nOn Windows, try using Windows Terminal or Putty instead.\n");
    }
    // --- Main loop ---
    while (1)
    {
        char* line = linenoise("> "); // Read line with editing & history
        if (line != NULL)
        {
            linenoiseHistoryAdd(line);
#ifdef SHELL_HISTORY_ENABLE
            linenoiseHistorySave(HISTORY_PATH);
#endif
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

