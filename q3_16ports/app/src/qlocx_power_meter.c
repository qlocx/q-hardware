/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "qlocx_defines.h"
#ifdef QLOCX_POWER_METER_ENABLED
#include "qlocx_i2c.h"
#include "qlocx_power_meter.h"

#include "nrf_delay.h"
#include "nrf_log.h"

#define INA219_I2C_ADDRESS             0x40

#define INA219_CONFIGURATION_ADDRESS   0x00
#define INA219_SHUNT_VOLTAGE_ADDRESS   0x01
#define INA219_BUS_VOLTAGE_ADDRESS     0x02
#define INA219_POWER_ADDRESS           0x03
#define INA219_CURRENT_ADDRESS         0x04
#define INA219_CALIBRATION_ADDRESS     0x05

// values from https://github.com/adafruit/Adafruit_INA219
static const uint32_t INA219_CAL_VALUE            = 4096;
static const uint32_t INA219_CURRENT_DIVIDER_MA   = 10;


static void qlocx_power_meter_set_power_off()
{

  uint8_t tx_data[3] = {0};

  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_POWERDOWN;

  tx_data[0] = INA219_CONFIGURATION_ADDRESS;
  tx_data[1] = (config >> 8) & 0x00ff;
  tx_data[2] = (config) & 0x00ff;

  qlocx_i2c_tx(INA219_I2C_ADDRESS, tx_data, sizeof(tx_data));

}

static void qlocx_power_meter_set_calibration_32v_2a()
{

  /* Set calibration */

  uint8_t tx_data[3] = {0};

  tx_data[0] = INA219_CALIBRATION_ADDRESS;
  tx_data[1] = (INA219_CAL_VALUE >> 8) & 0x00ff;
  tx_data[2] = (INA219_CAL_VALUE) & 0x00ff;

  qlocx_i2c_tx(INA219_I2C_ADDRESS, tx_data, sizeof(tx_data));

  /* Set config */

  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
                    INA219_CONFIG_GAIN_8_320MV |
                    INA219_CONFIG_BADCRES_12BIT |
                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

  tx_data[0] = INA219_CONFIGURATION_ADDRESS;
  tx_data[1] = (config >> 8) & 0x00ff;
  tx_data[2] = (config) & 0x00ff;

  qlocx_i2c_tx(INA219_I2C_ADDRESS, tx_data, sizeof(tx_data));

}

void qlocx_power_meter_init()
{
  NRF_LOG_INFO("Power measurement initializing.")
  qlocx_power_meter_set_power_off();
}

int16_t qlocx_power_meter_get_voltage_millivolts()
{

  // enable
  qlocx_power_meter_set_calibration_32v_2a();

  // delay to let us capture measurement
  nrf_delay_ms(20);

  // recieve data
  uint8_t rx_data[2] = {0};
  qlocx_i2c_rx(INA219_I2C_ADDRESS, INA219_BUS_VOLTAGE_ADDRESS, rx_data, sizeof(rx_data));
  uint16_t value = (rx_data[0] << 8) | (rx_data[1]);
  uint16_t result = ((int16_t)((value >> 3) * 4)) + QLOCX_POWER_METER_METER_BUS_VOLTAGE_DROP_MV;

  // disable
  qlocx_power_meter_set_power_off();

  return result;

}


uint16_t qlocx_power_meter_get_current_raw()
{

  // enable
  qlocx_power_meter_set_calibration_32v_2a();

  // delay to let us capture measurement
  nrf_delay_ms(20);

  // recieve data
  uint8_t rx_data[2] = {0};
  qlocx_i2c_rx(INA219_I2C_ADDRESS, INA219_CURRENT_ADDRESS, rx_data, sizeof(rx_data));
  uint16_t value = (rx_data[0] << 8) | (rx_data[1]);

  // disable
  qlocx_power_meter_set_power_off();

  return value;

}

uint16_t qlocx_power_meter_get_current_milliampere()
{
  return qlocx_power_meter_get_current_raw() / INA219_CURRENT_DIVIDER_MA;
}
#endif
