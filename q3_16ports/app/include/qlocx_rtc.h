/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_rtc.h
 *
 * The RTC module commuicates with the DS3231 to
 * keep track of and report the date and time
 * used by the board.
 */

#include "qlocx_defines.h"

#ifdef QLOCX_RTC_ENABLED
#ifndef QLOCX_RTC_H
#define QLOCX_RTC_H

#include "qlocx_date.h"

void qlocx_rtc_init();
void qlocx_rtc_get_date(qlocx_date_t* date);
void qlocx_rtc_set_date(qlocx_date_t* date);
void qlocx_rtc_print_current_date();

#endif
#endif
