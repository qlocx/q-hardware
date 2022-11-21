/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "qlocx_defines.h"
#ifdef QLOCX_RTC_ENABLED
#include "qlocx_date.h"
#include "qlocx_i2c.h"
#include "qlocx_config.h"
#include "qlocx_rtc.h"

#include "nrf_log.h"

#define DS3231_I2C_ADDRESS             0x68
#define DS3231_TIME_CAL_ADDRESS        0x00

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

/**
 * Initialize the RTC module.
 * This function should be called before
 * any member of this module is used.
 */
void qlocx_rtc_init()
{
  NRF_LOG_INFO("RTC initializing.");
  // check if a there is a pending date written to flash
  // if this memory address isn't empty, we use the written date
  // as the new date of the board
  qlocx_date_t empty_date = {0};
  uint8_t buffer[QLOCX_CONFIG_DATE_SIZE] = {0};
  qlocx_config_get_date(buffer);
  qlocx_date_t date = {0};
  memcpy(&date, buffer, sizeof(qlocx_date_t));
  if (memcmp(&date, &empty_date, sizeof(qlocx_date_t)) != 0)
  {
    qlocx_rtc_set_date(&date);
    qlocx_config_set_date(&empty_date);
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

  tx_data[0] = DS3231_TIME_CAL_ADDRESS;
  tx_data[1] = qlocx_rtc_dec_to_bcd(date->second);
  tx_data[2] = qlocx_rtc_dec_to_bcd(date->minute);
  tx_data[3] = qlocx_rtc_dec_to_bcd(date->hour);
  tx_data[4] = 0; // skip weekday
  tx_data[5] = qlocx_rtc_dec_to_bcd(date->day);
  tx_data[6] = qlocx_rtc_dec_to_bcd(date->month);
  tx_data[7] = qlocx_rtc_dec_to_bcd(date->year);

  qlocx_i2c_tx(DS3231_I2C_ADDRESS, tx_data, sizeof(tx_data));

}

/**
 * Get the current date.
 *
 * @param date the structure to copy the date to
 */
void qlocx_rtc_get_date(qlocx_date_t* date)
{

  uint8_t rx_data[7] = {0};

  qlocx_i2c_rx(DS3231_I2C_ADDRESS, DS3231_TIME_CAL_ADDRESS, rx_data, sizeof(rx_data));

  date->second = qlocx_rtc_bcd_to_dec(rx_data[0]);
  date->minute = qlocx_rtc_bcd_to_dec(rx_data[1]);
  date->hour   = qlocx_rtc_bcd_to_dec(rx_data[2]);
  // skip weekday
  date->day    = qlocx_rtc_bcd_to_dec(rx_data[4]);
  date->month  = qlocx_rtc_bcd_to_dec(rx_data[5]);
  date->year   = qlocx_rtc_bcd_to_dec(rx_data[6]);

}
#endif
