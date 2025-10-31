#include "esp32xx_hwspi.h"
#include "esp_log.h"

#include <stdlib.h>
#include <string.h>

#define HWSPI_SPI_HOSTS (3)
#define HWSPI_MAX_SS_PIN (48)
#define HWSPI_MAX_SLAVES (3*HWSPI_SPI_HOSTS) // 3 CS for each SPI(1,2,3)

#define SPI_MAP_BITS_PER_ENTRY   6
#define SPI_MAP_MAX_ENTRIES      9
#define SPI_MAP_HOST_BITS        2


typedef struct {
    // dynamic array will be here.
    spi_device_handle_t * hwspi_handle;
   /* LSB
    *  dev 1
    * |0000 00|00 0000|0000 00|00 0000|0000 00|
    *                                   1  2  3 - host device counters
    * |0000 00|0000 00|00 0000|0000 00|00|00|00|0000 MSB
    */
    uint64_t pin_ss_map;

} slave_container_t;

slave_container_t * spi_container = NULL;

/*
 * @brief Sets bits in the spi map, that are related to the 
 * slave select pin of the spi device.
 *
 * @warn Each element can be only 6 bit long!
 * @warn The first pin is on LSB, counting from 0.
 *
 * See comment above for the map description
 */
static inline void spi_map_set_pin(uint64_t *map, uint8_t index, uint8_t pin)
{
    uint64_t mask = (0x3F) << (index * SPI_MAP_BITS_PER_ENTRY);
    *map = (*map & ~mask) | ((uint64_t)(pin & 0x3F) << (index * SPI_MAP_BITS_PER_ENTRY));
}
/*
 * @brief Returns bits in the spi map, that are related to the 
 * slave select pin of the spi device.
 *
 * @warn Each element can be only 6 bit long!
 * @warn The first pin is on LSB, counting from 0.
 *
 * See comment above for the map description
 */
static inline uint8_t spi_map_get_pin(uint64_t *map, uint8_t index)
{
    return (*map >> (index * SPI_MAP_BITS_PER_ENTRY)) & 0x3F;
}
/*
 * @brief Sets bits in the spi map, that are related to the 
 * number of registered devices.
 *
 * @warn host counting starts from 0!
 *
 * See comment above for the map description
 */
static inline void spi_map_host_set(uint64_t *map, uint8_t host, uint8_t value)
{
    uint8_t bitpos = SPI_MAP_MAX_ENTRIES * SPI_MAP_BITS_PER_ENTRY + (host) * SPI_MAP_HOST_BITS;
    uint64_t mask = ((1ULL << SPI_MAP_HOST_BITS) - 1) << bitpos;
    *map = (*map & ~mask) | ((uint64_t)(value & 0x03) << bitpos);
;
}
/*
 * @brief Returns bits in the spi map, that are related to the 
 * number of registered devices.
 *
 * @warn host counting starts from 0!
 *
 * See comment above for the map description
 */
static inline uint8_t spi_map_host_get(uint64_t *map, uint8_t host)
{
    return (*map >> (SPI_MAP_MAX_ENTRIES * SPI_MAP_BITS_PER_ENTRY + (host)*2));
}
/*
 * @brief Increments bits in the spi map, that are related to the 
 * number of registered devices.
 *
 * @warn host counting starts from 0!
 *
 * See comment above for the map description
 */
static inline void spi_map_host_increment(uint64_t *map, uint8_t host)
{
    uint8_t val = spi_map_host_get(map, host);
    if (val < 3)
    {
        spi_map_host_set(map, host, (val+1));
    }
}
/*
 * @brief Decrements bits in the spi map, that are related to the 
 * number of registered devices.
 *
 * @warn host counting starts from 0!
 *
 * See comment above for the map description
 */
static inline void spi_map_host_decrement(uint64_t * map, uint8_t host)
{
    uint8_t val = spi_map_host_get(map, host);
    if (val > 0)
    {
        spi_map_host_set(map, host, (val-1));
    }
}

/*
 * @brief Checks how much SPI devices are already registered
 *
 */
static uint8_t slaves_registered()
{
    uint8_t ret = 0;
    for (uint8_t p = 0; p < HWSPI_SPI_HOSTS; p++)
    {
        ret += spi_map_host_get(&spi_container->pin_ss_map, p);
    }
    return ret;
}

/*
 * @brief Checks if the host was already registered and if
 * there are any conflicts between spi dev creation requests.
 *
 * There can be only 3 SPI devices that means that only 3 configurations
 * of MISO,MOSI,SCK can be possible.
 *
 */
static uint8_t host_assert(hwspi_ctx_t *c)
{
    uint8_t asserted_host = c->spi_host; 
    // TODO: 
    // 1) Finish the assertion
    // 2) Finish the create function with error codes returns
    // 3) Finish dev open()
    // 4) Write code for the driver handle destruction on close()
    // 5) Write the code for SPI RX/TX and for ioctl management
    // 6) Test.
}

/*
 * @brief Creates new spi device in ESP-IDF and 
 * registers it in the handles list.
 * 
 */
static int8_t device_create(hwspi_ctx_t *c, spi_bus_config_t *buscfg,
                            spi_device_interface_config_t *devcfg,
                            spi_device_handle_t *ret_handle)
{
    esp_err_t ret;
    uint8_t total_devices = 0;
    if (spi_container == NULL) 
    {
        spi_container = calloc(1, sizeof(slave_container_t));
        if (!spi_container) return -1;

        spi_container->hwspi_handle = NULL;
        spi_container->pin_ss_map = 0;
    }
    else
    {
        total_devices = slaves_registered();
    }

    spi_device_handle_t *new_handles =
        realloc(spi_container->hwspi_handle, (total_devices + 1) * sizeof(spi_device_handle_t));
    if (!new_handles) return -2;
    spi_container->hwspi_handle = new_handles;

    ret = spi_bus_initialize(c->spi_host, buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
    if (ret != ESP_OK) return -3;

    spi_device_handle new_handle;
    ret = spi_bus_add_device(c->spi_host, devcfg, &new_handle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
    if (ret != ESP_OK) return -4;

    spi_container->hwspi_handle[total_devices] = new_handle;

    spi_map_set_pin(&spi_container->pin_ss_map, total_devices, devcfg->spics_io_num);
    spi_map_host_increment(&spi_container->pin_ss_map, c->spi_host);

    if (ret_handle != NULL)
    {
        *ret_handle = new_handle;
    }
    return 0;
}

static int hwspi_open(void *priv)
{
    hwspi_ctx_t *c = (hwspi_ctx_t*)priv;
    
    spi_bus_config_t buscfg = {
        .miso_io_num = c->spi_miso,
        .mosi_io_num = c->spi_mosi,
        .sclk_io_num = c->spi_sck,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = c->spi_tx_size,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = c->spi_freq,
        .mode = 0,                              
        .spics_io_num = c->spi_ss,             
        .queue_size = c->spi_queue_size,
    };
    int8_t state = device_create(c, &buscfg, &devcfg, (spi_device_handle_t*)NULL);
        

    return 0;
}

static int hwspi_close(void *priv)
{
    hwspi_ctx *c = (hwspi_ctx_t*)priv;
    //stop SPI here
    esp_err_t ret = spi_bus_remove_device();
    return 0;
}


static ssize_t hwspi_read(void *priv, void *buf, size_t count)
{
    hwspi_ctx *c = (hwspi_ctx_t*)priv;

    

}

static ssize_t hwspi_write(void *priv, const void *buf, size_t count)
{
    hwspi_ctx *c = (hwspi_ctx_t*)priv;

}

static int hwspi_ioctl(void *priv, int req, void *arg)
{
    hwspi_ctx *c = (hwspi_ctx_t*)priv;
    switch (req) 
    {
        case HWSPI_IOCTL_GET_INFO:
        {

            return 0;
        }
        case HWSPI_IOCTL_SET_ISR:
        {

            return 0;
        }
        case HWSPI_IOCTL_SET_BUS:
        {

            return 0;
        }
        case HWSPI_IOCTL_SET_DMA:
        {

            return 0;
        }
        case HWSPI_IOCTL_SET_SPI_FREQ:
        {

            return 0;
        }
        default:
            return -1;
    }
}

/*
 * @brief VFS driver function table
 */
const DevFileOps hwspi_ops = {
    .read  = hwspi_read,
    .write = hwspi_write,
    .ioctl = hwspi_ioctl,
    .open  = hwspi_open,
    .close = hwspi_close,
};

