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
    HWSPI_IOCTL_GET_INFO = 0x100, //<-- pointer to get the output, returns hwspi_info_t
    HWSPI_IOCTL_SET_ISR,          //<-- 1 to use ISR, 0 to use polling 
    HWSPI_IOCTL_SET_BUS,          //<-- 1 to aquire bus, 0 to release
    HWSPI_IOCTL_SET_DMA,          //<-- 1 to enable DMA, to to disable DMA
    HWSPI_IOCTL_SET_SPI_FREQ,     //<-- 0 for default, other uint32 value for freq
};

enum {
    HWSPI_ERROR_PORT_ALREADY_REGISTERED = -1,
    HWSPI_ERROR_DEVICE_ALREADY_REGISTERED,
    HWSPI_ERROR_UNKNOWN_PORT,
    HWSPI_ERROR_UNKNOWN_DEVICE,
    HWSPI_ERROR_MAX_DEVICES_REACHED,
    HWSPI_ERROR_PORT_NOT_FOUND,
    HWSPI_ERROR_DEVICE_NOT_FOUND,
};

typedef struct {
    esp_err_t spi_state;
} hwspi_info_t;

typedef struct {
    uint8_t spi_host;
    uint8_t spi_miso;
    uint8_t spi_mosi;
    uint8_t spi_sck;
    uint8_t spi_ss;
    uint32_t spi_freq;
    uint32_t spi_tx_size;
    uint32_t spi_queue_size;
} hwspi_ctx_t;

extern const DevFileOps hwspi_ops;

#ifdef __cplusplus
}
#endif

#endif
