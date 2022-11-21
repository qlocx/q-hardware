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

/* PORTS FOR QLOCX BOARD Q3 */

/**
 * OUT1   = gpio 9
 * OUT2   = gpio 8
 * OUT3   = gpio 7
 * OUT4   = gpio 6
 * OUT5   = gpio 5
 * OUT6   = gpio 4
 * OUT7   = gpio 3
 * OUT8   = gpio 2
 * OUT9   = gpio 19
 * OUT10  = gpio 22
 * OUT11  = gpio 23
 * OUT12  = gpio 24
 * OUT13  = gpio 25
 * OUT14  = gpio 26
 * OUT15  = gpio 27
 * OUT16  = gpio 28
 * PWR    = gpio 14
 * SENSE  = gpio 20
 * RTCINT = gpio 30
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
  {19, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {22, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {23, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {24, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {25, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {26, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {27, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {28, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {14, QLOCX_PORT_OUTPUT, {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {20, QLOCX_PORT_INPUT,  {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  {30, QLOCX_PORT_INPUT,  {{0}}, NULL, QLOCX_PORT_STATUS_UNINITIALIZED},\
  }


#define QLOCX_PORTS_MINIMUM_TIMER_TICKS 5

void qlocx_ports_init();
void qlocx_ports_enable(uint8_t index, uint16_t duration, uint16_t delay);
uint8_t qlocx_ports_get_status(uint8_t index);

#endif
