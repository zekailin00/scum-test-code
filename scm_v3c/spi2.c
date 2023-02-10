#include "spi2.h"

#include "scm3c_hw_interface.h"
#include "Memory_Map.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct node_t
{
    int handle;
    spi_pin_config_t config;
    struct node_t* child;
} node_t;

static int handle_count = 0;
static node_t* root = 0;
static node_t* last = 0;

void digitalWrite(int pin, int high_low);

uint8_t digitalRead(int pin);

node_t* get_node(int handle)
{
    node_t *node = root;

    // Find the device specified by the handle
    while(node != 0)
    {
        if (node->handle == handle)
            break;
        else
            node = node->child;
    }
    return node;
}

int open(spi_pin_config_t *pin_config, spi_mode_t* mode)
{
    uint32_t gpi;
    uint32_t gpo;
    node_t* node;

    gpi = 1 << pin_config->MISO |
          mode->gpi_extra;
    gpo = 1 << pin_config->CS |
          1 << pin_config->MOSI |
          1 << pin_config->SCLK |
          mode->gpo_extra;
    
    node = malloc(sizeof(node_t));

    GPI_enables(gpi);
    GPO_enables(gpo);

    // FIXME: not sure if it is required?
    // Program analog scan chain (update GPIO configs)
    analog_scan_chain_write();
    analog_scan_chain_load();

    digitalWrite(pin_config->MOSI, 0);    // reset low
    digitalWrite(pin_config->SCLK, 0);    // reset low
    //FIXME: should it be high?
    digitalWrite(pin_config->CS, 0);    // reset low

    node->handle = handle_count++;
    memcpy(&node->config, pin_config, sizeof(spi_pin_config_t));
    node->child = 0;

    if (root == 0)
    {
        root = node; 
        last = node->child;
    }
    else 
    {
        last->child = node;
        last = node;
    }
    printf("using spi2\n");
    return node->handle;
}

int ioctl(int handle, int request, void *argp)
{
    node_t* node;
    
    node = get_node(handle);
    if (node == 0)
        return INVALID_HANDLE;

    switch (request)
    {
    case SPI_SELECT:
        digitalWrite(node->config.CS, 0);
        break;
    case SPI_DESELECT:
        digitalWrite(node->config.CS, 1);
        break;        
    default:
        return INVALID_IOCTL_REQ;
    }

    return 0;
}

int write(int handle, const unsigned char write_byte)
{
    int bit;
    node_t* node;

    printf("SPI write begin\n");

    node = get_node(handle);
    if (node == 0)
        return INVALID_HANDLE;

	// clock low at the beginning
	digitalWrite(node->config.SCLK, 0);

	// sample at falling edge
	for (bit = 7; bit >= 0; bit--)
    {
        digitalWrite(node->config.SCLK, 1);
        digitalWrite(node->config.MOSI, (write_byte & (1<<bit)) != 0);
        digitalWrite(node->config.SCLK, 0);
	}

    // set data out to 0
	digitalWrite(node->config.MOSI, 0);

    return 1; // always writes 1 byte.
}

int read(int handle, unsigned char* byte)
{
	int bit;
    node_t* node;

    *byte = 0;
    node = get_node(handle);
    if (node == 0)
        return INVALID_HANDLE;

	// sample at falling edge
	for (bit = 7; bit >= 0; bit--)
    {
        digitalWrite(node->config.SCLK, 1);
        digitalWrite(node->config.SCLK, 0);
        *byte |= digitalRead(node->config.MISO) << bit;		
	}

    return 1; // always reads 1 byte.
}

int close(int handle)
{
    //TODO: disable GPIOs and free the node corresponding to the handle?
}