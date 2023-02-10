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
    uint32_t data_rate; // TODO: not used yet.
    uint32_t buffer_size; // TODO: not used yet.
    uint16_t gpi_extra; // Additional gpi pins that needs to be enabled in addition to the 4 SPI pins
    uint16_t gpo_extra; // Additional gpo pins that needs to be enabled in addition to the 4 SPI pins
} spi_mode_t;

/**
 * Reads a byte to the SPI peripheral specified by handle.
 * @param handle The SPI peripheral handle returned from open().
 * @param byte The pointer to a buffer of a byte.
 * @return The number of bytes read, which is always 1 when successful.
 *      If operation fails, a negative value is returned indicating the error type.
*/
int read(int handle, unsigned char* byte);
//TODO:? int read(int handle, void *buf, int count);

/**
 * Writes a byte to the SPI peripheral specified by handle.
 * @param handle The SPI peripheral handle returned from open().
 * @param write_byte The value of the byte to be written into the SPI peripheral.
 * @return The number of bytes written, which is always 1 when successful.
 *      If operation fails, a negative value is returned indicating the error type.
*/
int write(int handle, const unsigned char write_byte);
//TODO:? int write(int fd, const void *buf, int count);

/**
 * Closes a SPI peripheral.
 * @param handle The SPI peripheral handle returned from open().
 * @return Non-negative value when successful.
 *      If operation fails, a negative value is returned indicating the error type.
*/
int close(int handle);

/**
 * Opens a SPI peripheral.
 * @param pin_config A pointer to spi_pin_config_t specifying the four pins of SPI.
 * @param mode A pointer to spi_mote_t specifying how the peripheral is used.
 * @return Non-negative handle representing the SPI peripheral.
 *      If operation fails, a negative value is returned indicating the error type.
*/
int open(spi_pin_config_t *pin_config, spi_mode_t* mode);

/**
 * IO control for SPI.
 * @param handle The SPI peripheral handle returned from open().
 * @param request A device-depend request code.
 * @param argp A pointer to addtional arguments for requests. 
 *      If a request does not have additional arguments, the value is 0.
 * @return Non-negative value when successful.
 *      If operation fails, a negative value is returned indicating the error type.
*/
int ioctl(int handle, int request, void *argp);

#endif