/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_i2c.h
 *
 * The I2C module handles the I2C communications
 * with the slave devices on the board.
 */

#ifndef QLOCX_I2C_H
#define QLOCX_I2C_H

#define QLOCX_I2C_SDA_PIN 16
#define QLOCX_I2C_SCL_PIN 17

#include <stdint.h>

void qlocx_i2c_init();
void qlocx_i2c_tx(uint8_t address, uint8_t *data, uint8_t length);
void qlocx_i2c_rx(uint8_t address, uint8_t rx_addr, uint8_t *data, uint8_t length);

#endif
