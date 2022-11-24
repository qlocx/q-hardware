/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_power_meter.h
 *
 * The power module communicates with the INA219 via I2C
 * to get the power, voltage and current consumed by
 * the board.
 */

#ifdef QLOCX_POWER_METER_ENABLED
#ifndef QLOCX_POWER_METER_METER_H
#define QLOCX_POWER_METER_METER_H

#include <stdint.h>

#define QLOCX_POWER_METER_METER_BUS_VOLTAGE_DROP_MV 380
#define INA219_CONFIG_GAIN_MASK         0x1800
#define INA219_CONFIG_BADCRES_MASK      0x0780
#define INA219_CONFIG_SADCRES_MASK      0x0078
#define INA219_CONFIG_MODE_MASK         0x0007

enum {
  /* 0-16V Range */
  INA219_CONFIG_BVOLTAGERANGE_16V   = 0x0000,
  /* 0-32V Range */
  INA219_CONFIG_BVOLTAGERANGE_32V   = 0x2000,
};

enum {
  /* Gain 1, 40mV Range */
  INA219_CONFIG_GAIN_1_40MV         = 0x0000,
  /* Gain 2, 80mV Range */
  INA219_CONFIG_GAIN_2_80MV         = 0x0800,
  /* Gain 4, 160mV Range */
  INA219_CONFIG_GAIN_4_160MV        = 0x1000,
  /* Gain 8, 320mV Range */
  INA219_CONFIG_GAIN_8_320MV        = 0x1800
};

enum {
  INA219_CONFIG_BADCRES_9BIT       =       (0x0000), 
  /* 9-bit bus res  = 0 ... 511 */
  INA219_CONFIG_BADCRES_10BIT      =       (0x0080),
  /* 10-bit bus res = 0 ... 1023 */
  /* 11-bit bus res = 0 ... 2047 */
  INA219_CONFIG_BADCRES_11BIT      =       0x0100,
  /* 12-bit bus res = 0 ... 4097 */
  INA219_CONFIG_BADCRES_12BIT       =      0x0180,
};

enum {
    INA219_CONFIG_SADCRES_9BIT_1S_84US     = (0x0000),  // 1 x 9-bit shunt sample
    INA219_CONFIG_SADCRES_10BIT_1S_148US   = (0x0008),  // 1 x 10-bit shunt sample
    INA219_CONFIG_SADCRES_11BIT_1S_276US   = (0x0010),  // 1 x 11-bit shunt sample
    INA219_CONFIG_SADCRES_12BIT_1S_532US   = (0x0018),  // 1 x 12-bit shunt sample
    INA219_CONFIG_SADCRES_12BIT_2S_1060US  = (0x0048),	 // 2 x 12-bit shunt samples averaged together
    INA219_CONFIG_SADCRES_12BIT_4S_2130US  = (0x0050),  // 4 x 12-bit shunt samples averaged together
    INA219_CONFIG_SADCRES_12BIT_8S_4260US  = (0x0058),  // 8 x 12-bit shunt samples averaged together
    INA219_CONFIG_SADCRES_12BIT_16S_8510US = (0x0060),  // 16 x 12-bit shunt samples averaged together
    INA219_CONFIG_SADCRES_12BIT_32S_17MS   = (0x0068),  // 32 x 12-bit shunt samples averaged together
    INA219_CONFIG_SADCRES_12BIT_64S_34MS   = (0x0070),  // 64 x 12-bit shunt samples averaged together
    INA219_CONFIG_SADCRES_12BIT_128S_69MS =  (0x0078),  // 128 x 12-bit shunt samples averaged together
};


/* Values for operating mode. */
 enum {   
  INA219_CONFIG_MODE_POWERDOWN            = 0x0000,
  INA219_CONFIG_MODE_SVOLT_TRIGGERED      = 0x0001,
  INA219_CONFIG_MODE_BVOLT_TRIGGERED      = 0x0002,
  INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED  = 0x0003,
  INA219_CONFIG_MODE_ADCOFF               = 0x0004,
  INA219_CONFIG_MODE_SVOLT_CONTINUOUS     = 0x0005,
  INA219_CONFIG_MODE_BVOLT_CONTINUOUS     = 0x0006,
  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS = 0x0007
};

void qlocx_power_meter_init();
int16_t qlocx_power_meter_get_voltage_millivolts();
uint16_t qlocx_power_meter_get_current_milliampere();

#endif
#endif

