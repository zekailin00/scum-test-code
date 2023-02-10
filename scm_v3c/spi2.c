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

void spi_digitalWrite(int pin, int high_low) {
    if (high_low) {
        GPIO_REG__OUTPUT |= (1 << pin);
	}
    else {
        GPIO_REG__OUTPUT &= ~(1 << pin);
	}
}

uint8_t spi_digitalRead(int pin) {
	uint8_t i = 0;
	i = (GPIO_REG__INPUT&(1 << pin)) >> pin;
	return i;
}

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

    // Program analog scan chain (update GPIO configs)
    analog_scan_chain_write();
    analog_scan_chain_load();

    spi_digitalWrite(pin_config->MOSI, 0);    // reset low
    spi_digitalWrite(pin_config->SCLK, 0);    // reset low
    spi_digitalWrite(pin_config->CS, 0);    // reset low

    node->handle = handle_count++;
    memcpy(&node->config, pin_config, sizeof(spi_pin_config_t));
    node->child = 0;

    if (root == 0)
    {
        root = node; 
        last = node;
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
        spi_digitalWrite(node->config.CS, 0);
        break;
    case SPI_DESELECT:
        spi_digitalWrite(node->config.CS, 1);
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
	spi_digitalWrite(node->config.SCLK, 0);

	// sample at falling edge
	for (bit = 7; bit >= 0; bit--)
    {
        spi_digitalWrite(node->config.SCLK, 1);
        spi_digitalWrite(node->config.MOSI, (write_byte & (1<<bit)) != 0);
        spi_digitalWrite(node->config.SCLK, 0);
	}

    // set data out to 0
	spi_digitalWrite(node->config.MOSI, 0);

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
        spi_digitalWrite(node->config.SCLK, 1);
        spi_digitalWrite(node->config.SCLK, 0);
        *byte |= spi_digitalRead(node->config.MISO) << bit;		
	}

    return 1; // always reads 1 byte.
}

int close(int handle)
{
    node_t *node = root;
    node_t *prev_node = 0;

    // Find the device specified by the handle
    while(node != 0)
    {
        if (node->handle == handle)
            break;
        else
        {
            prev_node = node;
            node = node->child;
        }
    }

    if (node == 0)
    {
        return INVALID_HANDLE;
    }

    if (node == root && node == last)
    { // only one node, which is both root and last node.
        root = 0;
        last = 0;
    }
    else if (node == root)
    { // node is root
        node = node->child;
    }
    else if (node == last)
    { // node is the last node
        last = prev_node;
    }
    else
    { // regular case
        prev_node->child = node->child;
    }

    free(node);
    return 0;
}