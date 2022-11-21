/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_ports.h
 *
 * The ports module is used to send current through the
 * ports of the board.
 */

#ifndef QLOCX_PORTS_H
#define QLOCX_PORTS_H

#include "app_timer.h"
#include "qlocx_defines.h"
#include <stdint.h>

enum {
  QLOCX_PORT_OUTPUT,
  QLOCX_PORT_INPUT
};

enum {
  QLOCX_PORT_STATUS_ENABLED,
  QLOCX_PORT_STATUS_DISABLED,
  QLOCX_PORT_STATUS_UNINITIALIZED
};

typedef struct qlocx_port_s {
  uint8_t gpio;
  uint8_t direction;
  app_timer_t timer_data;
  app_timer_id_t timer;
  int status;
  uint16_t timeout;
} qlocx_port_t;

/* PORTS FOR QLOCX BOARD Q1 */
#ifdef QLOCX_BOARD_Q1
/**
 * OUT1   = gpio 9
 * OUT2   = gpio 8
 * OUT3   = gpio 7
 * OUT4   = gpio 6
 * OUT5   = gpio 5
 * OUT6   = gpio 4
 * OUT7   = gpio 3
 * OUT8   = gpio 2
 * RELAY1 = gpio 11
 * RELAY2 = gpio 12
 * SENSE  = gpio 20
 */
#define QLOCX_PORTS_INIT {                                               \
  {9,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {8,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {7,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {6,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {5,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {4,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {3,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {2,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {11, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {12, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {20, QLOCX_PORT_INPUT,  {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
}
#endif
/* END OF PORTS FOR QLOCX BOARD Q1 */

/* PORTS FOR QLOCX BOARD Q1A */
#ifdef QLOCX_BOARD_Q1A
/**
 * RELAY1 = gpio 11
 * RELAY2 = gpio 12
 * SENSE  = gpio 20
 */
#define QLOCX_PORTS_INIT {                                               \
  {11, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {12, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {20, QLOCX_PORT_INPUT,  {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
}
#endif
/* END OF PORTS FOR QLOCX BOARD Q1A */

/* PORTS FOR QLOCX BOARD Q1B */
#ifdef QLOCX_BOARD_Q1B
/**
 * OUT1   = gpio 9
 * OUT2   = gpio 8
 * OUT3   = gpio 7
 * OUT4   = gpio 6
 * OUT5   = gpio 5
 * OUT6   = gpio 4
 * OUT7   = gpio 3
 * OUT8   = gpio 2
 */
#define QLOCX_PORTS_INIT {                                               \
  {9,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {8,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {7,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {6,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
}
#endif
/* END OF PORTS FOR QLOCX BOARD Q1B */

/* PORTS FOR QLOCX BOARD Q2 */
#ifdef QLOCX_BOARD_Q2
/**
 * OUT    = gpio 9
 * PWR    = gpio 11
 * SENSE  = gpio 20
 */
#define QLOCX_PORTS_INIT {                                               \
  {9,  QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {11, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {20, QLOCX_PORT_INPUT,  {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
}
#endif
/* END OF PORTS FOR QLOCX BOARD Q2 */

#define QLOCX_PORTS_MINIMUM_TIMER_TICKS 5

void qlocx_ports_init();
void qlocx_ports_enable(uint8_t index, uint16_t duration, uint16_t delay);
uint8_t qlocx_ports_get_status(uint8_t index);

#endif
