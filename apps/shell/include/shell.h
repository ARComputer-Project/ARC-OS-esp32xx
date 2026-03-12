#ifndef SHELL_APP
#define SHELL_APP

#include "esp_console.h"
#include "fs_internal.h"
#include "target.h"

#define HISTORY_PATH INTFS_MOUNT_PATH "history.txt"

extern const esp_console_cmd_t shell_cmds[];
extern const size_t shell_cmds_size;

#ifdef SHELL_ENABLE
void shell_task(void* params);
#else
inline void shell_task(void* params) {}
#endif

int cmd_led_on(int argc, char** argv);
int cmd_led_off(int argc, char** argv);
int cmd_reboot(int argc, char** argv); 

#endif
