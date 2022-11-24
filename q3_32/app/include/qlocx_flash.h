/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_flash.h
 *
 * The flash module occupies flash memory and handles
 * writes from and reads to the non volatile data
 * storage.
 */

#ifndef QLOCX_FLASH_H
#define QLOCX_FLASH_H

#include "qlocx_defines.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

void qlocx_flash_set(size_t page_index, size_t byte_index, void* src, size_t num_bytes);
void qlocx_flash_get(size_t page_index, size_t byte_index, void* dest, size_t num_bytes);
int  qlocx_flash_cmp(size_t page_index, size_t byte_index, void* buf, size_t num_bytes);
bool qlocx_flash_is_empty(size_t page_index, size_t byte_index, size_t num_bytes);
void qlocx_flash_erase_page(size_t page_index);
void qlocx_flash_init();

#endif
