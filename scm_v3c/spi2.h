#include <stdint.h>

#ifndef __SPI_LIB
#define __SPI_LIB
/**
 * TODO:
 * 1. Currently, it only samples at falling edge.
*/

// error return values
#define INVALID_HANDLE -1 // handle is not valid
#define INVALID_IOCTL_REQ -2 // ioctl request is not valid

// ioctl requests
#define SPI_SELECT 1 // select SPI device
#define SPI_DESELECT 0 // deselect SPI device

typedef struct spi_pin_config_t{
    uint8_t MISO;
    uint8_t MOSI;
    uint8_t SCLK;
    uint8_t CS;
} spi_pin_config_t;

typedef struct spi_mode_t{
    uint32_t data_rate;
    uint32_t buffer_size;
    uint16_t gpi_extra;
    uint16_t gpo_extra;
} spi_mode_t;

/**
 * Reads a byte to the SPI device specified by handle.
*/
int read(int handle, unsigned char* byte);
//TODO:? int read(int handle, void *buf, int count);

/**
 * Writes a byte to the SPI device specified by handle.
*/
int write(int handle, const unsigned char write_byte);
//TODO:? int write(int fd, const void *buf, int count);

/**
 * Closes a SPI device.
*/
int close(int handle);

/**
 * Opens a SPI device.
*/
int open(spi_pin_config_t *pin_config, spi_mode_t* mode);

/**
 * IO control for SPI.
*/
int ioctl(int handle, int request, void *argp);

#endif