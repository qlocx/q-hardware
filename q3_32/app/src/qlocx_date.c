/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "qlocx_date.h"
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "nrf_log.h"

void qlocx_date_init()
{
  NRF_LOG_INFO("Date initializing.");
}

void qlocx_date_from_isostring(qlocx_date_t* date, uint8_t* str)
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  sscanf(
    (char*) str,
    "20%02d-%02d-%02dT%02d:%02d:%02dZ",
    &year,
    &month,
    &day,
    &hour,
    &minute,
    &second);
  date->year   = year;
  date->month  = month;
  date->day    = day;
  date->hour   = hour;
  date->minute = minute;
  date->second = second;
}

void qlocx_date_to_isostring(uint8_t* str, qlocx_date_t* date)
{
  int year   = date->year;
  int month  = date->month;
  int day    = date->day;
  int hour   = date->hour;
  int minute = date->minute;
  int second = date->second;
  sprintf(
    (char*) str,
    "20%02d-%02d-%02dT%02d:%02d:%02dZ",
    year,
    month,
    day,
    hour,
    minute,
    second);
}

/**
 * The qlocx_date_cmp function compares two dates
 * a and b.
 *
 * @param a the first date to compare
 * @param b the second date to compare
 * @return An integer less than, equal to, or greater
 * than zero if a is found, respectively to be
 * less than, to match, or to be greater than b.
 */
int qlocx_date_cmp(qlocx_date_t* a, qlocx_date_t* b)
{
  return memcmp(a, b, sizeof(qlocx_date_t));
}

