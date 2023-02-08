#include <string.h>
#include <stdio.h>

#include "scm3c_hw_interface.h"
#include "memory_map.h"
#include "optical.h"
#include "ads1299-2.h"
//=========================== defines =========================================

#define CRC_VALUE         (*((unsigned int *) 0x0000FFFC))
#define CODE_LENGTH       (*((unsigned int *) 0x0000FFF8))

//=========================== variables =======================================

typedef struct {
    ads_data_t ads_measurement[100];
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

//=========================== main ============================================

int main(void) {
    uint32_t i, j;
    unsigned char print_reg;
    uint32_t Nsample = 100;
    uint8_t rreg;

    memset(&app_vars,0,sizeof(app_vars_t));
    
    initialize_mote();
    crc_check();
    perform_calibration();
    
	ADS_initialize();
	ADS_RESET();
    ADS_SDATAC();
    print_reg = ADS_RREG(0x00);
	printf("ID: %x\r\n",  print_reg); // should return 3E
	print_reg = ADS_RREG(0x0c);
	printf("channel8: %x\r\n",  print_reg); // print the config off the ADS
    ADS_WREG(0x0c, 0x60);               // enable channel 8
    print_reg = ADS_RREG(0x0c);         // confirm channel 8 is enabled
	printf("channel8: %x\r\n",  print_reg);
    ADS_WREG(0x03, 0xE0);             // change the config on ADS
    print_reg = ADS_RREG(0x03);       // confirm the config on ADS
	printf("CONFIG3: %x\n",  print_reg);
		
    for (rreg = 0x00; rreg < 0x18; rreg = rreg + 0x01) {
      print_reg = ADS_RREG(rreg);
      printf("%x\r\n", print_reg);
    }

    ADS_RREGS(0x00, 0x17);
    
    ADS_START();
    ADS_RDATAC();
		
	for (i = 0; i < 10; i++);
		
    for (i = 0; i < Nsample; i++) {
      read_ads_register(&app_vars.ads_measurement[i]);
    }

    for (i = 0; i < Nsample; i++) {
      // printf("%x\n",app_vars.ads_measurement[i].config);
	    for (j = 7; j < 8; j++) {
		    printf("0x%x\r\n", app_vars.ads_measurement[i].channel[j]);
	    }
    }

	// ADS_SDATAC();
	// for (i = 0; i < 10; i++);
	
    // for (rreg = 0x00; rreg < 0x18; rreg = rreg + 0x01) {
    //   print_reg = ADS_RREG(rreg);
    //   printf("%x\n", print_reg);
    // }

    
    // ADS_STOP();

  // while(1){
		// print_reg = ADS_RREG(0x00);
		// printf("ID: %x\n",  print_reg);
			// spi_write(0x11);
//			printf("0x11\n");
		 //      ADS_initialize();  
//			read_ads_register(&app_vars.ads_measurement);
//        log_ads_data(&app_vars.ads_measurement);

//        for (i = 0; i < 1000000; i++);
  //  }
}

//=========================== public ==========================================

//=========================== private =========================================
