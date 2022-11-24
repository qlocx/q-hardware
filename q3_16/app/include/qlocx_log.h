/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

 /**
  * @file qlocx_log.h
  */

#ifndef QLOCX_LOG_H
#define QLOCX_LOG_H

#include "qlocx_date.h"
#include <stdint.h>

#define QLOCX_LOG_FLASH_PAGE_INDEX 0
#define QLOCX_LOG_ENTRY_SIZE 16

  /**
   * The maximum number of entries the log can hold.
   */
#ifdef QLOCX_BOARD_Q2
#define QLOCX_LOG_MAX_ENTRIES 10
#else
#define QLOCX_LOG_MAX_ENTRIES (QLOCX_FLASH_PAGE_SIZE / QLOCX_LOG_ENTRY_SIZE)
#endif

void    qlocx_log_init();
void    qlocx_log_add_entry(uint8_t* hash);
void    qlocx_log_clear_until_date(qlocx_date_t* date);
void    qlocx_log_clear_until_hash(uint8_t* hash);
void    qlocx_log_clear_all();
size_t  qlocx_log_size();
void    qlocx_log_cpy(void* dest);


#endif
