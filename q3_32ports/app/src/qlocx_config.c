/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "qlocx_flash.h"
#include "qlocx_config.h"
#include "nrf_log.h"

void qlocx_config_init()
{
  NRF_LOG_INFO("Config initializing.");
}

void qlocx_config_get_public_key(void* dest)
{
  qlocx_flash_get(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_PUBLIC_KEY_BYTE_INDEX,
      dest,
      QLOCX_CONFIG_PUBLIC_KEY_SIZE);
}

void qlocx_config_set_public_key(void* src)
{
  qlocx_flash_set(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_PUBLIC_KEY_BYTE_INDEX,
      src,
      QLOCX_CONFIG_PUBLIC_KEY_SIZE);
}

void qlocx_config_get_private_key(void* dest)
{
  qlocx_flash_get(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_PRIVATE_KEY_BYTE_INDEX,
      dest,
      QLOCX_CONFIG_PUBLIC_KEY_SIZE);
}

void qlocx_config_set_private_key(void* src)
{
  qlocx_flash_set(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_PRIVATE_KEY_BYTE_INDEX,
      src,
      QLOCX_CONFIG_PRIVATE_KEY_SIZE);
}

void qlocx_config_get_device_name(void* dest)
{
  qlocx_flash_get(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_DEVICE_NAME_BYTE_INDEX,
      dest,
      QLOCX_CONFIG_DEVICE_NAME_SIZE);
}

void qlocx_config_set_device_name(void* src)
{
  qlocx_flash_set(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_DEVICE_NAME_BYTE_INDEX,
      src,
      QLOCX_CONFIG_DEVICE_NAME_SIZE);
}

void qlocx_config_get_sense_config(void* dest)
{
  qlocx_flash_get(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_SENSE_CONFIG_BYTE_INDEX,
      dest,
      QLOCX_CONFIG_SENSE_CONFIG_SIZE);
}

void qlocx_config_set_sense_config(void* src)
{
  qlocx_flash_set(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_SENSE_CONFIG_BYTE_INDEX,
      src,
      QLOCX_CONFIG_SENSE_CONFIG_SIZE);
}

void qlocx_config_get_date(void* dest)
{
  qlocx_flash_get(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_DATE_BYTE_INDEX,
      dest,
      QLOCX_CONFIG_DATE_SIZE);
}

void qlocx_config_set_date(void* src)
{
  qlocx_flash_set(
      QLOCX_CONFIG_FLASH_PAGE_INDEX,
      QLOCX_CONFIG_DATE_BYTE_INDEX,
      src,
      QLOCX_CONFIG_DATE_SIZE);
}

