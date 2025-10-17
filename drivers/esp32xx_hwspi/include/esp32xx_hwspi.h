#ifndef ESP32XX_DRIVER_HWSPI_H
#define ESP32XX_DRIVER_HWSPI_H

#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdatomic.h>

#include "esp_attr.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"

#include "driver_structs.h"

/* Compatibility with C++ code */
#ifdef __cplusplus
extern "C" {
#endif

enum {
    HWSPI_IOCTL_GET_INFO = 0x100,
};

typedef struct {
    uint8_t spi_host;
    uint8_t spi_miso;
    uint8_t spi_mosi;
    uint8_t spi_sck;
    uint8_t spi_ss;
} hwspi_info_t;

typedef struct {
    uint8_t spi_host;
    uint8_t spi_miso;
    uint8_t spi_mosi;
    uint8_t spi_sck;
    uint8_t spi_ss;
    uint32_t sck_freq;
} hwspi_ctx_t;

extern const DevFileOps hwspi_ops;

#ifdef __cplusplus
}
#endif

#endif
