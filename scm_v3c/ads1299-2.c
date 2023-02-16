#include "ads1299-2.h"
#include "Memory_Map.h"
#include <stdio.h>
#include "spi2.h"


// SPI bus definitions
#define CS_PIN      11
#define CLK_PIN     14
#define DIN_PIN     13 // Used when reading data from the IMU thus a SCuM input
#define DATA_PIN    12 // Used when writing to the IMU thus a SCuM output
// ADS pin definitions
#define RST_PIN     15  // need to check IO (output)
#define DRDY_PIN    3  // need to check IO (input)
#define ADS_DVDD 	7	// ADS1299 is powered by SCuM's GPIO pin 7, supplies 1.8V

int spi_handle = 0;

void digitalWrite(int pin, int high_low) {
	// printf("wrote pin: %d high_low =: %d\r\n", pin, high_low);
    if (high_low) {
        GPIO_REG__OUTPUT |= (1 << pin);
	}
    else {
        GPIO_REG__OUTPUT &= ~(1 << pin);
	}
}

uint8_t digitalRead(int pin) {
	uint8_t i = 0;
	int clk_pin = CLK_PIN;
	i = (GPIO_REG__INPUT&(1 << pin)) >> pin;
	// printf("i = %d\n", i);
	return i;
}

void spi_write(const unsigned char write_byte )
{
    write(spi_handle, write_byte);
}

unsigned char spi_read(){
    unsigned char read_byte;
    read(spi_handle, &read_byte);
    return read_byte;
}


void ADS_initialize() {
    int t;
    spi_mode_t spi_mode;
    spi_pin_config_t spi_config;
    uint16_t gpi;
    uint16_t gpo;

    // Hex nibble 4: 0x8 = 0b1000 
    //  Pin 3 (DRDY)
    gpi = 0x0008 | GPI_enables_read();

    // Hex nibble 1: 0x8  = 0b1000 =
    //  Pin 15 (ADS_RESET)
    // Hex nibble 3: 0x8 = 0b1000 =
    //  Pin 7 (ADS_DVDD 1.8V)
    gpo = 0x8080 | GPO_enables_read();

    GPI_enables(gpi);
    GPO_enables(gpo);

    // Program analog scan chain (update GPIO configs)
    analog_scan_chain_write();
    analog_scan_chain_load();

    spi_config.CS = CS_PIN;
    spi_config.MISO = DIN_PIN;
    spi_config.MOSI = DATA_PIN;
    spi_config.SCLK = CLK_PIN;

    spi_handle = open(&spi_config, &spi_mode);
    
    if (spi_handle < 0)
    {
        printf("Open SPI failed.\n");
        exit();
    }

    // Enable power to the ADS1299
	digitalWrite(ADS_DVDD, 1);
    // toggle reset pin
    digitalWrite(RST_PIN, 0);

    // cortex clock 2MHz(0.5us)
    // power up ~32ms
    for (t = 0; t < 65000; t++);

	printf("Initialized ADS1299\r\n");
}

// System Commands
void ADS_WAKEUP() {
    int t;
    
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _WAKEUP);
    ioctl(spi_handle, SPI_CS, 1);
    for (t = 0; t < 10; t++);
    // must wait 4 tCLK before sending another commands
}

// only allow to send WAKEUP after sending STANDBY
void ADS_STANBY() {
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _STANDBY);
    ioctl(spi_handle, SPI_CS, 1);
}

// reset all the registers to defaut settings
void ADS_RESET() {
    int t;
    
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _RESET);

    // must wait 18 tCLK to execute this command
    for (t = 0; t < 20; t++);
    
    ioctl(spi_handle, SPI_CS, 1);

    printf("ADS reset complete\n");
}

// start data conversion
void ADS_START() {
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _START);
    ioctl(spi_handle, SPI_CS, 1);
}

// stop data conversion
void ADS_STOP() {
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _STOP);
    ioctl(spi_handle, SPI_CS, 1);
}

void ADS_RDATAC() {
    int t;
    
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _RDATAC);
    ioctl(spi_handle, SPI_CS, 1);
    
    // must wait 4 tCLK after executing thsi command
    for (t = 0; t < 10; t++);
}

void ADS_SDATAC() {
    int t;
    printf("SDATAC begin\n");
    
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, _SDATAC);
    ioctl(spi_handle, SPI_CS, 1);
    
    // must wait 4 tCLK after executing thsi command
    for (t = 0; t < 10; t++);
}

unsigned char ADS_RREG(unsigned char addr) {
    unsigned char opcode1 = addr + 0x20;
    unsigned char read_reg;
    
    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, opcode1);
    write(spi_handle, 0x00);
    read(spi_handle, &read_reg);
    ioctl(spi_handle, SPI_CS, 1);
    
    return read_reg;
}

void ADS_WREG(unsigned char addr, unsigned char val) {
	unsigned char opcode1 = addr + 0x40;

    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, opcode1);
    write(spi_handle, 0x00);
    write(spi_handle, val);
    ioctl(spi_handle, SPI_CS, 1);
}

void ADS_RREGS(unsigned char addr, unsigned char NregminusOne) {
    unsigned char opcode1 = addr + 0x20;
    unsigned char read_reg;
	unsigned char i;

    ioctl(spi_handle, SPI_CS, 0);
    write(spi_handle, opcode1);
    write(spi_handle, NregminusOne);
    
	for (i = 0; i <= NregminusOne; i++) {
        read(spi_handle, &read_reg);
		printf("%x\n", read_reg);
	}
    ioctl(spi_handle, SPI_CS, 1);
}

void read_ads_register(ads_data_t* ads_measurement) {
	unsigned char read_reg;
	int nchan = 8;
	int i, j;
	int32_t read_24bit;
    unsigned char data_ready;

//	ADS_START();

    do read(spi_handle, &data_ready);
    while (data_ready);

    ioctl(spi_handle, SPI_CS, 0);
	read_24bit = 0;
	for (i = 0; i < 3; i++) {
        read(spi_handle, &read_reg);
		read_24bit = (read_24bit << 8) | read_reg;
		// printf("%x", read_reg);
	}
	// printf("\n");
	ads_measurement->config = read_24bit;
	// printf("config = %x\n", read_24bit);
	for (j = 0; j < nchan; j++) {
		read_24bit = 0;
		for (i = 0; i < 3; i++) {
            read(spi_handle, &read_reg);
			read_24bit = (read_24bit << 8) | read_reg;
			// printf("%x", read_reg);
		}
		// printf("\n");
		if (read_24bit >> 23) {
			read_24bit |= 0xFF000000;
		}
		else {
			read_24bit &= 0x00FFFFFF;
		}
		ads_measurement->channel[j] = read_24bit;
		// printf("channel[%d] = %x\n", j, read_24bit);
	}
    ioctl(spi_handle, SPI_CS, 1);

	// printf("%x\n", ads_measurement->config);
	// for (j = 0; j < nchan; j++) {
	// 	printf("%x\n", ads_measurement->channel[j]);
	// }
}