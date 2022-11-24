/***************************************************************************
 * qlocx_ioexp.c 				                                                
 *											
 *  
 *  
 *                                                                                      
 * Functional level: 1									
 *											
 * Prepared: Dirk Handzic							
 *																					
 * 											
 * Rev	Date	Author	Ext int. (Y/N) 	Reason/description
 * 0.1  220901  DH	Y        	Initial version
 *
 *  
 * PC9535 I/O expansion handler 										
 ***************************************************************************/


#include "qlocx_defines.h"
#include "qlocx_i2c.h"
#include "qlocx_config.h"
#include "qlocx_ioexp.h"
#include "nrf_log.h"
#include <stdio.h>

//********************************* Definitions *************************************
#define PCA9535_BASE_ADDR 0x20
#define IOEXP_INPUTS      0x22
#define IOEXP_17_24       0x20
#define IOEXP_25_32       0x21

#define PCA9535_INPUT_0   0x00
#define PCA9535_INPUT_1   0x01
#define PCA9535_OUTPUT_0  0x02
#define PCA9535_OUTPUT_1  0x03
#define PCA9535_POLINV_0  0x04
#define PCA9535_POLINV_1  0x05
#define PCA9535_CONFIG_0  0x06
#define PCA9535_CONFIG_1  0x07

//********************************* Internal functions ******************************
uint8_t   qlocx_ioexp_read_register(uint8_t addr, uint8_t device)
{
  uint8_t rx_data = 0;

  qlocx_i2c_rx(device, addr, &rx_data, 1);
  return(rx_data);
}


void  qlocx_ioexp_write_register(uint8_t addr, uint8_t val, uint8_t device)
{
  uint8_t tx_data[2] = {0};

  tx_data[0] = addr;
  tx_data[1] = val;
  qlocx_i2c_tx(device, tx_data, 2);
}

//********************************* External functions ******************************

void qlocx_ioexp_init(void)
{
    uint8_t tx_data[4] = {0};

    NRF_LOG_INFO("I/O expansion initializing.")

    // Clear output register
    qlocx_ioexp_write_register(PCA9535_OUTPUT_0, 0x00, IOEXP_17_24);
    // Write configuration to configuration registers ports 17-24
    tx_data[0] = PCA9535_CONFIG_0;
    tx_data[1] = 0x00;                                  // port 0 output
    tx_data[2] = 0xFF;                                  // port 1 input
    qlocx_i2c_tx(IOEXP_17_24, tx_data, 3);
    
    // Clear output register
    qlocx_ioexp_write_register(PCA9535_OUTPUT_0, 0x00, IOEXP_25_32);
    // Write configuration to configuration registers ports 25-32
    tx_data[0] = PCA9535_CONFIG_0;
    tx_data[1] = 0x00;                                  // port 0 output
    tx_data[2] = 0xFF;                                  // port 1 input
    qlocx_i2c_tx(IOEXP_25_32, tx_data, 3);

    // Write configuration to configuration registers inputs 1 - 16
    tx_data[0] = PCA9535_CONFIG_0;
    tx_data[1] = 0xFF;                                  // port 0 input
    tx_data[2] = 0xFF;                                  // port 1 input
    qlocx_i2c_tx(IOEXP_INPUTS, tx_data, 3);
}

// Index 0 - 31 
// returns status for the input
bool  qlocx_ioexp_read_input(uint8_t index)
{
uint32_t result;
uint32_t mask;
bool status;

    mask = 1 << index;

    if(index < 16){
        result = qlocx_ioexp_read_register(PCA9535_INPUT_0, IOEXP_INPUTS);
        result |= (qlocx_ioexp_read_register(PCA9535_INPUT_1, IOEXP_INPUTS) << 8);
    }
    else if(index < 24){
        
        result = (qlocx_ioexp_read_register(PCA9535_INPUT_1, IOEXP_17_24) << 16);
    }
    else{
         result = (qlocx_ioexp_read_register(PCA9535_INPUT_1, IOEXP_25_32) << 24);
    }
    if(mask & result){
        status = true;
    }
    else{ 
        status = false;
    }
    return(status);
}

// Index 0 - 15 
// set output
void  qlocx_ioexp_write_output(uint8_t index, bool state)
{

uint8_t mask;
uint8_t status;

    if(index > 7){     
        mask = 1 << (index - 8);
    }
    else{
         mask = 1 << index;
    }
      
    if(index < 8){
        status = qlocx_ioexp_read_register(PCA9535_OUTPUT_0,IOEXP_17_24);         // Get present status
        if(state){
            mask |= status;                                                       // set bit
        }
        else{
            mask = status & ~(mask);                                              // clear bit
        }
        qlocx_ioexp_write_register(PCA9535_OUTPUT_0, mask, IOEXP_17_24);
    }
    else{
        status = qlocx_ioexp_read_register(PCA9535_OUTPUT_0,IOEXP_25_32);
         if(state){
            mask |= status;                                                       // set bit
        }
        else{
            mask = status & ~(mask);                                              // clear bit
        }
        qlocx_ioexp_write_register(PCA9535_OUTPUT_0, mask, IOEXP_25_32);
    }
    
}

// returns status for the door status inputs
uint32_t  qlocx_ioexp_read_all_inputs(void)
{
uint32_t result;
  

    
    result = qlocx_ioexp_read_register(PCA9535_INPUT_0, IOEXP_INPUTS) & 0xFF;
    result |= (qlocx_ioexp_read_register(PCA9535_INPUT_1, IOEXP_INPUTS) << 8);  
    result |= (qlocx_ioexp_read_register(PCA9535_INPUT_1, IOEXP_17_24) << 16);
    result |= (qlocx_ioexp_read_register(PCA9535_INPUT_1, IOEXP_25_32) << 24);
    
    return(result);
}