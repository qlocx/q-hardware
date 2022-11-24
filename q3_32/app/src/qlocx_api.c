/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "qlocx_api.h"
#include "qlocx_date.h"
#include "qlocx_log.h"
#include "qlocx_ports.h"
#include "qlocx_defines.h"
#include "qlocx_power_meter.h"
#include "qlocx_config.h"
#include "qlocx_rtc.h"

#include "nrf_log.h"

void qlocx_api_init()
{
  NRF_LOG_DEBUG("API initializing.");
}

/**
 * Send current through a port on the board.
 *
 * @param context the current BLE connection
 * @return true on success, false on failure
 */
bool qlocx_api_open_port(qlocx_ble_t *context)
{

  if (((context->rx_length - 14) % 3) != 0) return false;

#ifdef QLOCX_RTC_ENABLED
  qlocx_date_t valid_from = {
    .year   = context->rx_buffer[1],
    .month  = context->rx_buffer[2],
    .day    = context->rx_buffer[3],
    .hour   = context->rx_buffer[4],
    .minute = context->rx_buffer[5],
    .second = context->rx_buffer[6],
  };

  qlocx_date_t valid_until = {
    .year   = context->rx_buffer[7],
    .month  = context->rx_buffer[8],
    .day    = context->rx_buffer[9],
    .hour   = context->rx_buffer[10],
    .minute = context->rx_buffer[11],
    .second = context->rx_buffer[12],
  };

  qlocx_date_t current_date = {0};
  qlocx_rtc_get_date(&current_date);

  if (qlocx_date_cmp(&current_date, &valid_until) > 0)
  {
    return false;
  }

  if (qlocx_date_cmp(&current_date, &valid_from) < 0)
  {
    return false;
  }
#endif

  //uint8_t user_id   = request[13];

  for (size_t i = 14; i < context->rx_length; i += 3)
  {
    uint8_t port_id   = context->rx_buffer[i];
    uint16_t duration = context->rx_buffer[i+1] * 100;
    uint16_t delay    = context->rx_buffer[i+2] * 100;
    qlocx_ports_enable(port_id, duration, delay);
  }

  return true;

}

/**
 * Check if a port is part of an open or a closed circuit.
 *
 * @param context the current BLE connection
 * @return true on success, false on failure
 */
bool qlocx_api_get_port_status(qlocx_ble_t *context)
{

  if (context->rx_length != 2) return false;

  context->tx_buffer[0] = qlocx_ports_get_status(context->rx_buffer[1]);
  context->tx_length = 1;

  return true;

}

/**
 * Write the current input voltage to the tx_buffer.
 * The voltage is given in millivolts and written
 * as a big endian 16 bit integer.
 *
 * @return true on success, false on failure
 */
#ifdef QLOCX_POWER_METER_ENABLED
bool qlocx_api_get_battery_status(qlocx_ble_t *context)
{
  if (context->rx_length != 1) return false;
  uint16_t voltage = qlocx_power_meter_get_voltage_millivolts();
  context->tx_buffer[0] = (voltage >> 8) & 0xff;
  context->tx_buffer[1] = voltage & 0xff;
  context->tx_length = 2;
  return true;
}
#endif

/**
 * Write the current date to the tx_buffer.
 * The date is written as a 6 byte @ref qlocx_date_t.
 *
 * @param context the current BLE connection
 * @return true on success, false on failure
 */
#ifdef QLOCX_RTC_ENABLED
bool qlocx_api_get_date(qlocx_ble_t *context)
{
  if (context->rx_length != 1) return false;
  qlocx_date_t date;
  qlocx_rtc_get_date(&date);
  memcpy(context->tx_buffer, &date, sizeof(date));
  context->tx_length = sizeof(date);

  return true;
}
#endif

/**
 * Update the current date used by the board.
 * The date should be given as a 6 byte @ref qlocx_date_t.
 *
 * @param context the current BLE connection
 * @return true on success, false on failure
 */
#ifdef QLOCX_RTC_ENABLED
bool qlocx_api_set_date(qlocx_ble_t *context)
{
  if (context->rx_length != 7) return false;
  qlocx_date_t date = {
    .year   = context->rx_buffer[1],
    .month  = context->rx_buffer[2],
    .day    = context->rx_buffer[3],
    .hour   = context->rx_buffer[4],
    .minute = context->rx_buffer[5],
    .second = context->rx_buffer[6],
  };
  qlocx_rtc_set_date(&date);
  context->tx_length = 0;
  return true;
}
#endif

/**
 * Write the current log entries to the tx_buffer.
 *
 * @param context the current BLE connection
 * @return true on success, false on failure
 */
bool qlocx_api_get_logs(qlocx_ble_t *context)
{
  if (context->rx_length != 2) return false;
  context->tx_length = qlocx_log_size();
  qlocx_log_cpy(context->tx_buffer);
  return true;
}

/**
 * Remove all log entries up until and including
 * the given date in rx_buffer.
 *
 * @return true on success, false on failure
 */
bool qlocx_api_purge_logs_until_date(qlocx_ble_t *context)
{
  NRF_LOG_INFO("rx_length %d",context->rx_length);
 if(context->rx_length != 7) return false;


 qlocx_date_t date = {
    .year   = context->rx_buffer[1],
    .month  = context->rx_buffer[2],
    .day    = context->rx_buffer[3],
    .hour   = context->rx_buffer[4],
    .minute = context->rx_buffer[5],
    .second = context->rx_buffer[6],
  };

  qlocx_log_clear_until_date(&date);

  return true;
}

/**
 * Remove all log entries up until and including
 * the given date in rx_buffer.
 *
 * @return true on success, false on failure
 */
bool qlocx_api_purge_logs_until_hash(qlocx_ble_t *context)
{
//  if(context->rx_length != 7) return false;

NRF_LOG_INFO("rx length %d",context->rx_length);

  uint8_t hash[QLOCX_LOG_ENTRY_SIZE] = {0};
  // memcpy(hash, context->rx_buffer, )

  qlocx_log_clear_until_hash(hash);

  return true;
}

/**
 * Set the sense configuration.
 *
 * @return true on success, false on failure
 */
bool qlocx_api_set_sense_config(qlocx_ble_t *context)
{
  if (context->rx_length != 14) return false;
  // skip command and sense_pin
  uint8_t *sense_config = context->rx_buffer + 2;
  qlocx_config_set_sense_config(sense_config);
  return true;
}

/**
 * Set the sense configuration.
 *
 * @return true on success, false on failure
 */
bool qlocx_api_get_software_version(qlocx_ble_t *context)
{
  if (context->rx_length != 1) return false;
  context->tx_buffer[0] = 0;
  context->tx_buffer[1] = QLOCX_SOFTWARE_VERSION;
  context->tx_length = 2;
  return true;
}

/**
 * Check door lock status
 *
 * @param context the current BLE connection
 * @return true on success, false on failure
 */
bool qlocx_api_get_door_status(qlocx_ble_t *context)
{
uint32_t door_status;
uint32_t mask;
uint8_t i;

  if (context->rx_length != 1) return false;
  door_status = qlocx_ports_get_all_status();
  mask = 1;  
  for(i = 0; i < 32;i++){
      if(door_status & mask){
          context->tx_buffer[i] = 1;
       }
       else{
	  context->tx_buffer[i] = 0;	 	
       }
       mask <<= 1;			
  }	
  context->tx_length = 32;
  return true;
}

