/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "app_util_platform.h"
#include "nrf_drv_twi.h"
#include "qlocx_i2c.h"

#include "nrf_log.h"

#define TWI_INSTANCE_ID 0

static const nrf_drv_twi_t twi_instance = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

void qlocx_i2c_init()
{

  NRF_LOG_INFO("I2C initializing.");

  uint32_t err_code;
  
  const nrf_drv_twi_config_t twi_config =
  {
    .scl                = QLOCX_I2C_SCL_PIN,
    .sda                = QLOCX_I2C_SDA_PIN,
    .frequency          = NRF_TWI_FREQ_100K,
    .interrupt_priority = APP_IRQ_PRIORITY_HIGH
  };
  
  err_code = nrf_drv_twi_init(&twi_instance, &twi_config, NULL, NULL);

  APP_ERROR_CHECK(err_code);
  
  nrf_drv_twi_enable(&twi_instance);

}

void qlocx_i2c_tx(uint8_t address, uint8_t *data, uint8_t length)
{
  uint32_t err_code;
  err_code = nrf_drv_twi_tx(&twi_instance, address, data, length, false);
  APP_ERROR_CHECK(err_code);
}

void qlocx_i2c_rx(uint8_t address, uint8_t rx_address, uint8_t *data, uint8_t length)
{
  uint32_t err_code;
  err_code = nrf_drv_twi_tx(&twi_instance, address, &rx_address, 1, false);
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_twi_rx(&twi_instance, address, data, length);
  APP_ERROR_CHECK(err_code);
}

