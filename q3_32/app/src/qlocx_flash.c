/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "qlocx_flash.h"
#include <stdbool.h>
#include "app_error.h"
#include <stdlib.h>
#include "nrf_sdh_soc.h"
#include "nrf_log.h"

static void fs_evt_handler(nrf_fstorage_evt_t* p_evt)
{
  if (p_evt->result != NRF_SUCCESS) {
    APP_ERROR_CHECK(p_evt->result);
  }
}

NRF_FSTORAGE_DEF(nrf_fstorage_t qlocx_flash_instance) =
{
  .evt_handler    = fs_evt_handler,
  .start_addr     = QLOCX_FLASH_START_ADDR,
  .end_addr       = QLOCX_FLASH_END_ADDR,
};

static uint8_t qlocx_flash_buffer[8192] __attribute__((aligned(32)));

void qlocx_flash_init()
{
  NRF_LOG_INFO("Flash initializing.");
  uint32_t ret;
  ret = nrf_fstorage_init(
    &qlocx_flash_instance,
    &nrf_fstorage_sd,
    NULL
  );
  APP_ERROR_CHECK(ret);
  size_t src = QLOCX_FLASH_START_ADDR;
  uint8_t* dst = qlocx_flash_buffer;
  ret = nrf_fstorage_read(
    &qlocx_flash_instance,
    src,
    dst,
    sizeof(qlocx_flash_buffer)
  );
  APP_ERROR_CHECK(ret);
  while (nrf_fstorage_is_busy(&qlocx_flash_instance)) {
    sd_app_evt_wait();
  }
}

/**
 * Persist a given flash page, that is, store the given flash page of
 * qlocx_flash_buffer in physical flash.
 */
static void qlocx_flash_persist_page(uint8_t page_index)
{

  // erase the page first
  size_t num_pages = 1;
  uint32_t ret = nrf_fstorage_erase(
    &qlocx_flash_instance,
    QLOCX_FLASH_START_ADDR + QLOCX_FLASH_PAGE_SIZE * page_index,
    num_pages,
    NULL
  );
  APP_ERROR_CHECK(ret);

  size_t dst = QLOCX_FLASH_START_ADDR + QLOCX_FLASH_PAGE_SIZE * page_index;
  uint8_t* src = qlocx_flash_buffer + QLOCX_FLASH_PAGE_SIZE * page_index;

  ret = nrf_fstorage_write(
    &qlocx_flash_instance,
    dst,
    src,
    QLOCX_FLASH_PAGE_SIZE,
    NULL
  );
  APP_ERROR_CHECK(ret);

}

/**
 * Persist a given region, that is, store the given region of
 * qlocx_flash_buffer in physical flash.
 */
static void qlocx_flash_persist_region(uint8_t page_index, uint16_t byte_index, uint16_t num_bytes)
{

  size_t dst = QLOCX_FLASH_START_ADDR + QLOCX_FLASH_PAGE_SIZE * page_index + byte_index;
  uint8_t* src = qlocx_flash_buffer + QLOCX_FLASH_PAGE_SIZE * page_index + byte_index;


  // persist flash region
  uint32_t ret = nrf_fstorage_write(
    &qlocx_flash_instance,
    dst,
    src,
    num_bytes,
    NULL
  );
  APP_ERROR_CHECK(ret);

}


void qlocx_flash_erase_page(size_t page_index)
{

  // set the buffer to reflect the contents of flash
  memset(qlocx_flash_buffer + QLOCX_FLASH_PAGE_SIZE * page_index, 0xff, QLOCX_FLASH_PAGE_SIZE);

  // erase whole flash page
  size_t num_pages = 1;
  uint32_t ret = nrf_fstorage_erase(
    &qlocx_flash_instance,
    QLOCX_FLASH_START_ADDR + QLOCX_FLASH_PAGE_SIZE * page_index,
    num_pages,
    NULL
  );
  APP_ERROR_CHECK(ret);

}

void qlocx_flash_get(size_t page_index, size_t byte_index, void* dest, size_t num_bytes)
{
  uint8_t* src = qlocx_flash_buffer + page_index * QLOCX_FLASH_PAGE_SIZE + byte_index;
  memcpy(dest, src, num_bytes);
}

int qlocx_flash_cmp(size_t page_index, size_t byte_index, void* buf, size_t num_bytes)
{
  return memcmp(qlocx_flash_buffer + page_index * QLOCX_FLASH_PAGE_SIZE + byte_index, buf, num_bytes);
}

bool qlocx_flash_is_empty(size_t page_index, size_t byte_index, size_t num_bytes)
{
  for (size_t i = 0; i < num_bytes; i++)
  {
    if (qlocx_flash_buffer[page_index * QLOCX_FLASH_PAGE_SIZE + byte_index + i] != 0xff)
    {
      return false;
    }
  }
  return true;
}

void qlocx_flash_set(size_t page_index, size_t byte_index, void* src, size_t num_bytes)
{

  if (num_bytes == 0) return;

  uint8_t* current = qlocx_flash_buffer + page_index * QLOCX_FLASH_PAGE_SIZE + byte_index;

  // check if the flash region already match
  if (memcmp(current, src, num_bytes) == 0) return;

  // check if the flash was not written (meaning we don't need to erase the page)
  bool unwritten = true;
  for (size_t i = 0; i < num_bytes; i++) {
    if (current[i] != 0xff) {
      unwritten = false;
      break;
    }
  }

  // copy the data to the flash buffer
  memcpy(current, src, num_bytes);

  // if the flash was unwritten, we only need to persist the new region
  // otherwise, we need to update the whole page
  if (unwritten)
  {
    qlocx_flash_persist_region(page_index, byte_index, num_bytes);
  }
  else
  {
    qlocx_flash_persist_page(page_index);
  }

}

