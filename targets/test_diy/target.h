#ifndef TARGET_H
#define TARGET_H "TEST_DIY"

#include "esp32xx_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes and registers all devices for this specific target board.
 */
void target_init(void);

#ifdef __cplusplus
}
#endif


#endif