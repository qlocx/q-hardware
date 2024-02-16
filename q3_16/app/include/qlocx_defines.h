/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#ifndef QLOCX_DEFINES_H
#define QLOCX_DEFINES_H

/* NOTE: DO NOT INCLUDE OTHER HEADERS FROM HERE */

/* GLOBAL */

#define QLOCX_BOARD_Q3
#define QLOCX_SOFTWARE_VERSION 6

/* CONFIG */

#define QLOCX_CONFIG_FLASH_PAGE_INDEX         1
#define QLOCX_CONFIG_SENSE_CONFIG_BYTE_INDEX  0
#define QLOCX_CONFIG_SENSE_CONFIG_SIZE        12
#define QLOCX_CONFIG_PUBLIC_KEY_BYTE_INDEX    (QLOCX_CONFIG_SENSE_CONFIG_BYTE_INDEX + QLOCX_CONFIG_SENSE_CONFIG_SIZE)
#define QLOCX_CONFIG_PUBLIC_KEY_SIZE          32
#define QLOCX_CONFIG_PRIVATE_KEY_BYTE_INDEX   (QLOCX_CONFIG_PUBLIC_KEY_BYTE_INDEX + QLOCX_CONFIG_PUBLIC_KEY_SIZE)
#define QLOCX_CONFIG_PRIVATE_KEY_SIZE         32
#define QLOCX_CONFIG_DEVICE_NAME_BYTE_INDEX   (QLOCX_CONFIG_PRIVATE_KEY_BYTE_INDEX + QLOCX_CONFIG_PRIVATE_KEY_SIZE)
#define QLOCX_CONFIG_DEVICE_NAME_SIZE         32
#define QLOCX_CONFIG_DATE_BYTE_INDEX          (QLOCX_CONFIG_DEVICE_NAME_BYTE_INDEX + QLOCX_CONFIG_DEVICE_NAME_SIZE)
#define QLOCX_CONFIG_DATE_SIZE                6

/* FLASH */

/**
 * The page size of one flash page.
 * This value is defined by the nRF52832 and
 * should not be changed.
 */
#define QLOCX_FLASH_PAGE_SIZE 0x1000

/**
 * The start of the allocated flash, can be changed
 * if more flash would be needed.
 */
#define QLOCX_FLASH_START_ADDR 0x6d000

/**
 * The end of the allocated flash, this is dictated
 * by the end of the memory (we need space for the bootloader)
 * and should thus not be changed.
 *
 * Note: including the last byte.
 */
#define QLOCX_FLASH_END_ADDR   0x6efff

/* COMMON FUNCTIONALITY */

#define QLOCX_RTC_ENABLED
#define QLOCX_POWER_METER_ENABLED



#endif
