/*
 * Copyright (c) 2017-2020, Qlocx AB
 *
 * All rights reserved.
 *
 * 20210105   DH    change BSM to LSM
 * 20201216   DH    EEPROM handling rewritten
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
#define BSM_DSM     0x04
#define BSM_DSM     0x0C
#define FEDE        0x10
#define TCE         0x20
#define BSM_CLEAR   0xF3
#define BSM_LSM    0x0C

#define EEPROM_SINGLE_READ    0x22
#define EEPROM_SINGLE_WRITE   0x21
#define EEPROM_UPDATE         0x11
#define EEPROM_FIRST_CMD      0x00

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

uint8_t   qlocx_rtc_read_register(uint8_t addr)
{

  uint8_t rx_data = 0;

  qlocx_i2c_rx(RV3028_I2C_ADDRESS, addr, &rx_data, 1);
  return(rx_data);

}

void  qlocx_rtc_write_register(uint8_t addr, uint8_t val)
{
  uint8_t tx_data[2] = { 0 };

  tx_data[0] = addr;
  tx_data[1] = val;
  qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
}

bool  qlocx_rtc_waitforEEPROM(void)
{
  uint32_t timeout;

  timeout = 0;
  while ((qlocx_rtc_read_register(RV3028_STATUS) & EEBSY) && (timeout++ < 50000));
  return (timeout < 50000);
}

// Update all configuration RAM to EEPROM
// data point to an 8 byte array
char  qlocx_rtc_eeprom_update(uint8_t* data)
{
  uint8_t tx_data[8] = { 0 };
  uint8_t rx_data[7] = { 0 };
  uint8_t ctrl1;

  // write 1 to EERD                                      
  ctrl1 = qlocx_rtc_read_register(RV3028_CTRL1);
  ctrl1 |= EERD;
  qlocx_rtc_write_register(RV3028_CTRL1, ctrl1);

  qlocx_rtc_waitforEEPROM();

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

  do {
    qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_STATUS, rx_data, 1);
  } while ((rx_data[0] & EEBSY) == EEBSY);

  // write 0 to EERD
  tx_data[0] = RV3028_CTRL1;
  tx_data[1] = 0;
  qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, 2);
  qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_EEDATA, rx_data, 1);
  return(rx_data[0]);
}

// Read single byte from EEPROM address
uint8_t  qlocx_rtc_eeprom_read(uint8_t addr)
{
  bool success;
  uint8_t eeprom_data;
  uint8_t ctrl1;

  success = qlocx_rtc_waitforEEPROM();
  // write 1 to EERD                                      
  ctrl1 = qlocx_rtc_read_register(RV3028_CTRL1);
  ctrl1 |= EERD;
  qlocx_rtc_write_register(RV3028_CTRL1, ctrl1);

  // Set address
  qlocx_rtc_write_register(RV3028_EEADDR, addr);
  // First command
  qlocx_rtc_write_register(RV3028_EECMD, EEPROM_FIRST_CMD);
  // Second command
  qlocx_rtc_write_register(RV3028_EECMD, EEPROM_SINGLE_READ);

  if (!qlocx_rtc_waitforEEPROM()) {
    success = false;
  }
  eeprom_data = qlocx_rtc_read_register(RV3028_EEDATA);
  // write 0 to EERD
  ctrl1 = qlocx_rtc_read_register(RV3028_CTRL1);
  ctrl1 &= ~EERD;
  qlocx_rtc_write_register(RV3028_CTRL1, ctrl1);
  if (!success) {
    return(0xFF);
  }
  return(eeprom_data);
}

// Write a single byte to EEPROM address
bool qlocx_rtc_eeprom_write(uint8_t addr, uint8_t data)
{
  uint8_t ctrl1;
  bool success;

  success = qlocx_rtc_waitforEEPROM();
  // write 1 to EERD                                      
  ctrl1 = qlocx_rtc_read_register(RV3028_CTRL1);
  ctrl1 |= EERD;
  qlocx_rtc_write_register(RV3028_CTRL1, ctrl1);

  // Write config RAM register

  qlocx_rtc_write_register(addr, data);
  // Update EEPROM all configuration RAM -> EEPROM
  // First command
  qlocx_rtc_write_register(RV3028_EECMD, EEPROM_FIRST_CMD);
  // Second command
  qlocx_rtc_write_register(RV3028_EECMD, EEPROM_UPDATE);

  if (!qlocx_rtc_waitforEEPROM()) {
    success = false;
  }

  // write 0 to EERD
  ctrl1 = qlocx_rtc_read_register(RV3028_CTRL1);
  ctrl1 &= ~EERD;
  qlocx_rtc_write_register(RV3028_CTRL1, ctrl1);
  if (!qlocx_rtc_waitforEEPROM()) {
    success = false;
  }

  return(success);

}

/**
 * Initialize the RTC module.
 * This function should be called before
 * any member of this module is used.
 */
void qlocx_rtc_init()
{
  uint8_t eeprom_backup;
  bool success;

  // char status[20];

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
    // NRF_LOG_INFO("qlocx_rtc_set_date");
    qlocx_rtc_set_date(&date);

    // NRF_LOG_INFO("qlocx_config_set_date");
    qlocx_config_set_date(&empty_date);
    // NRF_LOG_INFO("RTC read backup.");
    eeprom_backup = qlocx_rtc_eeprom_read(RV3028_EE_BACKUP);
    eeprom_backup &= BSM_CLEAR;
    eeprom_backup |= BSM_LSM | FEDE;
    // NRF_LOG_INFO("RTC write backup: %d.", eeprom_backup);
    success = qlocx_rtc_eeprom_write(RV3028_EE_BACKUP, eeprom_backup);
    // NRF_LOG_INFO("RTC write backup done: %d.", success);
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
void  qlocx_rtc_set_date(qlocx_date_t* date)
{

  uint8_t tx_data[8] = { 0 };

  tx_data[0] = RV3028_TIME_CAL_ADDRESS;
  tx_data[1] = qlocx_rtc_dec_to_bcd(date->second);
  tx_data[2] = qlocx_rtc_dec_to_bcd(date->minute);
  tx_data[3] = qlocx_rtc_dec_to_bcd(date->hour);
  tx_data[4] = 0; // skip weekday
  tx_data[5] = qlocx_rtc_dec_to_bcd(date->day);
  tx_data[6] = qlocx_rtc_dec_to_bcd(date->month);
  tx_data[7] = qlocx_rtc_dec_to_bcd(date->year);


  qlocx_i2c_tx(RV3028_I2C_ADDRESS, tx_data, sizeof(tx_data));
}

/**
 * Get the current date.
 *
 * @param date the structure to copy the date to
 */
void qlocx_rtc_get_date(qlocx_date_t* date)
{

  uint8_t rx_data[7] = { 0 };

  qlocx_i2c_rx(RV3028_I2C_ADDRESS, RV3028_TIME_CAL_ADDRESS, rx_data, sizeof(rx_data));

  date->second = qlocx_rtc_bcd_to_dec(rx_data[0]);
  date->minute = qlocx_rtc_bcd_to_dec(rx_data[1]);
  date->hour = qlocx_rtc_bcd_to_dec(rx_data[2]);
  // skip weekday
  date->day = qlocx_rtc_bcd_to_dec(rx_data[4]);
  date->month = qlocx_rtc_bcd_to_dec(rx_data[5]);
  date->year = qlocx_rtc_bcd_to_dec(rx_data[6]);

}



#endif