/*
 * Copyright (c) 2017-2020, Qlocx AB
 *
 * All rights reserved.
 * 
 * 20201112   DH    Battery backup enabled on first clock/date set
 */

#include "qlocx_defines.h"
#ifdef QLOCX_RTC_ENABLED
#include "qlocx_date.h"
#include "qlocx_i2c.h"
#include "qlocx_config.h"
#include "qlocx_rtc.h"

#include "nrf_log.h"
#include <stdio.h>

#define RV3028_I2C_ADDRESS             0x52
#define RV3028_TIME_CAL_ADDRESS        0x00


// RV3028 registers
#define RV3028_STATUS      0x0e
#define RV3028_CTRL1       0x0f
#define RV3028_EEADDR      0x25
#define RV3028_EEDATA      0x26
#define RV3028_EECMD       0x27
#define RV3028_RAM_MIRROR  0x30
#define RV3028_EE_BACKUP   0x37

// RV3028 bits
#define EERD        0x08
#define EEBSY       0x80
#define BSM_DSM     0x0C
#define FEDE        0x10
#define TCE         0x20
#define BSM_CLEAR   0xF3
#define BSM_LSM    0x0C

// RV3028 bits
// #define EERD        0x08
// #define EEBSY       0x80
// #define BSM_DSM     0x04
// // #define BSM_DSM     0x0C
// #define FEDE        0x10
// #define TCE         0x20

// #define EEPROM_SINGLE_READ    0x22
// #define EEPROM_SINGLE_WRITE   0x21
// #define EEPROM_UPDATE         0x11
// #define EEPROM_FIRST_CMD      0x00

/**
 * Convert decimal to binary coded decimal.
 */
static uint8_t qlocx_rtc_dec_to_bcd(uint8_t val)
{
  return (((val / 10) * 16) + (val % 10));
}

/**
 * Convert binary coded decimal to decimal.
 */
static uint8_t qlocx_rtc_bcd_to_dec(uint8_t val)
{
  return (((val / 16) * 10) + (val % 16));
}

// Update all configuration RAM to EEPROM
// data point to an 8 byte array
char  qlocx_rtc_eeprom_update(uint8_t * data)
{
uint8_t tx_data[8] = {0};
uint8_t rx_data[7] = {0};
// uint8_t value;

    // write 1 to EERD assuming CTRL1 default POR value 0x00                                      
    tx_data[0] = RV3028_CTRL1;
    tx_data[1] = EERD;                              
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);

    do{
        qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
    }while((rx_data[0] & EEBSY) == EEBSY);

    // Write configuration to configuration RAM 0x30 to 0x37
    tx_data[0] = RV3028_EEADDR;
    tx_data[1] = 0x30;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, data, 8);
    // First command
    tx_data[0] = RV3028_EECMD;
    tx_data[1] = 0x00;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    // Second command
    tx_data[0] = RV3028_EECMD;
    tx_data[1] = 0x11;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);

    do{
        qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
    }while((rx_data[0] & EEBSY) == EEBSY);

    // write 0 to EERD
    tx_data[0] = RV3028_CTRL1;
    tx_data[1] = 0;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_EEDATA, rx_data, 1);   
    return(rx_data[0]);
}

// Write a single byte to EEPROM address
char  qlocx_rtc_eeprom_read(char addr)
{
uint8_t tx_data[8] = {0};
uint8_t rx_data[7] = {0};
// uint8_t value;

    // write 1 to EERD assuming CTRL1 default POR value 0x00                                      
    tx_data[0] = RV3028_CTRL1;
    tx_data[1] = EERD;                              
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);

    do{
        qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
    }while((rx_data[0] & EEBSY) == EEBSY);

    // Set address
    tx_data[0] = RV3028_EEADDR;
    tx_data[1] = addr;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    // First command
    tx_data[0] = RV3028_EECMD;
    tx_data[1] = 0x00;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    // Second command
    tx_data[0] = RV3028_EECMD;
    tx_data[1] = 0x22;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);

    do{
        qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
    }while((rx_data[0] & EEBSY) == EEBSY);

    // write 0 to EERD
    tx_data[0] = RV3028_CTRL1;
    tx_data[1] = 0;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_EEDATA, rx_data, 1);   
    return(rx_data[0]);
}

// Write a single byte to EEPROM address
void  qlocx_rtc_eeprom_write(char addr, char data)
{
uint8_t tx_data[8] = {0};
uint8_t rx_data[7] = {0};


    // write 1 to EERD
    tx_data[0] = RV3028_CTRL1;
    tx_data[1] = EERD;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);

    do{
        qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
    }while((rx_data[0] & EEBSY) == EEBSY);

    // Set address
    tx_data[0] = RV3028_EEADDR;
    tx_data[1] = addr;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    // Set data
    tx_data[0] = RV3028_EEDATA;
    tx_data[1] = data;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    // First command
    tx_data[0] = RV3028_EECMD;
    tx_data[1] = 0x00;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    // Second command
    tx_data[0] = RV3028_EECMD;
    tx_data[1] = 0x21;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
    do{
        qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
    }while((rx_data[0] & EEBSY) == EEBSY);

    // write 0 to EERD
    tx_data[0] = RV3028_CTRL1;
    tx_data[1] = 0;
    qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);   

}

/**
 * Initialize the RTC module.
 * This function should be called before
 * any member of this module is used.
 */
void qlocx_rtc_init()
{
  uint8_t eeprom_backup;

  NRF_LOG_INFO("RTC initializing.");
  // check if a there is a pending date written to flash
  // if this memory address isn't empty, we use the written date
  // as the new date of the board
  qlocx_date_t empty_date = { 0 };
  uint8_t buffer[QLOCX_CONFIG_DATE_SIZE] = { 0 };
  qlocx_config_get_date(buffer);
  qlocx_date_t date = { 0 };
  memcpy(&date, buffer, sizeof(qlocx_date_t));

  if (memcmp(&date, &empty_date, sizeof(qlocx_date_t)) != 0)
  {
    qlocx_rtc_set_date(&date);

    qlocx_config_set_date(&empty_date);
    eeprom_backup = qlocx_rtc_eeprom_read(RV3028_EE_BACKUP);
    eeprom_backup &= BSM_CLEAR;
    eeprom_backup |= BSM_LSM | FEDE;
    qlocx_rtc_eeprom_write(RV3028_EE_BACKUP, eeprom_backup);
  }
  else {
    qlocx_rtc_get_date(&date);
    NRF_LOG_INFO("RTC date %d.%d.%d.", date.year, date.month, date.day);
  }
}

/**
 * Set the current date.
 *
 * @param date the structure to read the date from
 */
void qlocx_rtc_set_date(qlocx_date_t* date)
{

  uint8_t tx_data[8] = {0};

  tx_data[0] = RV3028_TIME_CAL_ADDRESS;
  tx_data[1] = qlocx_rtc_dec_to_bcd(date->second);
  tx_data[2] = qlocx_rtc_dec_to_bcd(date->minute);
  tx_data[3] = qlocx_rtc_dec_to_bcd(date->hour);
  tx_data[4] = 0; // skip weekday
  tx_data[5] = qlocx_rtc_dec_to_bcd(date->day);
  tx_data[6] = qlocx_rtc_dec_to_bcd(date->month);
  tx_data[7] = qlocx_rtc_dec_to_bcd(date->year);

  NRF_LOG_INFO("qlocx_i2c_tx");
  qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, sizeof(tx_data));
}

/**
 * Get the current date.
 *
 * @param date the structure to copy the date to
 */
void qlocx_rtc_get_date(qlocx_date_t* date)
{

  uint8_t rx_data[7] = {0};

  qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_TIME_CAL_ADDRESS, rx_data, sizeof(rx_data));

  date->second = qlocx_rtc_bcd_to_dec(rx_data[0]);
  date->minute = qlocx_rtc_bcd_to_dec(rx_data[1]);
  date->hour   = qlocx_rtc_bcd_to_dec(rx_data[2]);
  // skip weekday
  date->day    = qlocx_rtc_bcd_to_dec(rx_data[4]);
  date->month  = qlocx_rtc_bcd_to_dec(rx_data[5]);
  date->year   = qlocx_rtc_bcd_to_dec(rx_data[6]);

}



#endif
