/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_date.h
 *
 * The date module contains date utilities
 * for the local date format of the board.
 */

#ifndef QLOCX_DATE_H
#define QLOCX_DATE_H

#include <stdint.h>

/**
 * The date format used by the board.
 */
typedef struct qlocx_date_s {
  /**
   * The year given in years after 2000 (0-255).
   */
  uint8_t year;
  /**
   * The month of the year (1-12).
   */
  uint8_t month;
  /**
   * The day of the month (1-31).
   */
  uint8_t day;
  /**
   * The hour of the day (0-23).
   */
  uint8_t hour;
  /**
   * The minute of the hour (0-59).
   */
  uint8_t minute;
  /**
   * The second of the minute (0-59).
   */
  uint8_t second;
} qlocx_date_t;

void qlocx_date_init();
void qlocx_date_to_isostring(uint8_t *str, qlocx_date_t *date);
void qlocx_date_from_isostring(qlocx_date_t *date, uint8_t *str);
int qlocx_date_cmp(qlocx_date_t *a, qlocx_date_t *b);

#endif
