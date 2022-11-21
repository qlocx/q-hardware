/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "nrf_log.h"
#include "qlocx_flash.h"
#include "qlocx_log.h"
#include "qlocx_rtc.h"
#include <stdint.h>
#include <stdlib.h>


#define HASH_SIZE 8

static size_t qlocx_log_num_entries = 0;

/**
 * Initialize the log module.
 * This function should be called before any of the members
 * of this module is used.
 */
void qlocx_log_init()
{
  NRF_LOG_INFO("Log initializing.");
  size_t page_index = QLOCX_LOG_FLASH_PAGE_INDEX;
  size_t byte_index = 0;
  // count the number of entries
  while (!qlocx_flash_is_empty(page_index, byte_index, QLOCX_LOG_ENTRY_SIZE))
  {
    qlocx_log_num_entries++;
    // if the flash is full, we need to erase all entries
    // and start at entry 0
    if (qlocx_log_num_entries == QLOCX_LOG_MAX_ENTRIES)
    {
      qlocx_flash_erase_page(QLOCX_LOG_FLASH_PAGE_INDEX);
      qlocx_log_num_entries = 0;
      break;
    }
    byte_index += QLOCX_LOG_ENTRY_SIZE;
  }
  NRF_LOG_INFO("%d log entries in flash. Total size: %d", qlocx_log_num_entries, qlocx_log_num_entries * QLOCX_LOG_ENTRY_SIZE);
}

/**
 * Add a log entry to the flash storage.
 *
 * @param hash A 16 byte hash to store, or NULL if
 * no hash should be stored
 */
void qlocx_log_add_entry(uint8_t* hash)
{

  uint8_t entry[QLOCX_LOG_ENTRY_SIZE] = { 0 };

  qlocx_date_t date = { 0 };
#ifdef QLOCX_RTC_ENABLED
  qlocx_rtc_get_date(&date);
#endif

  // copy current date onto entry
  memcpy(entry, &date, sizeof(date));

  // if the hash is present, copy it to the entry
  if (hash)
  {
    memcpy(entry + sizeof(date), hash, HASH_SIZE);
  }

  qlocx_flash_set(
    QLOCX_LOG_FLASH_PAGE_INDEX,
    qlocx_log_num_entries * QLOCX_LOG_ENTRY_SIZE,
    entry,
    QLOCX_LOG_ENTRY_SIZE);

  qlocx_log_num_entries++;

  if (qlocx_log_num_entries == QLOCX_LOG_MAX_ENTRIES)
  {
    qlocx_flash_erase_page(QLOCX_LOG_FLASH_PAGE_INDEX);
    qlocx_log_num_entries = 0;
  }

  NRF_LOG_INFO("Totalt antal logs i flash: %d", qlocx_log_num_entries);

}

// TODO: implementera qlocx_clear_until_hash

/**
 * Step through the log, and keep entries which are stored
 * on a date later than the given date. Throw away the other
 * entries.
 *
 * @param date the date to compare the entries with
 */
void qlocx_log_clear_until_date(qlocx_date_t* date)
{

  static uint8_t buffer[QLOCX_LOG_MAX_ENTRIES * QLOCX_LOG_ENTRY_SIZE];
  int saved_entries = 0;

  // loop through all available log entries
  for (int i = 0; i < QLOCX_LOG_MAX_ENTRIES; i++)
  {
    size_t page_index = QLOCX_LOG_FLASH_PAGE_INDEX;
    size_t byte_index = QLOCX_LOG_ENTRY_SIZE * i;
    // if the date of the entry is less than or equal to the date
    // we're searching for, we skip it
    if (qlocx_flash_cmp(page_index, byte_index, date, sizeof(date)) <= 0) continue;
    // if the entry is empty, we skip it
    if (qlocx_flash_is_empty(page_index, byte_index, sizeof(date))) continue;
    // save entry
    qlocx_flash_get(page_index, byte_index, buffer + saved_entries * QLOCX_LOG_ENTRY_SIZE, QLOCX_LOG_ENTRY_SIZE);
    saved_entries++;
  }

  qlocx_flash_erase_page(QLOCX_LOG_FLASH_PAGE_INDEX);
  qlocx_flash_set(
    QLOCX_LOG_FLASH_PAGE_INDEX,
    0,
    buffer,
    saved_entries * QLOCX_LOG_ENTRY_SIZE);

  qlocx_log_num_entries = saved_entries;
}

void qlocx_log_clear_until_hash(uint8_t* hash) {
  // static uint8_t buffer[QLOCX_LOG_MAX_ENTRIES * QLOCX_LOG_ENTRY_SIZE];
  // int saved_entries = 0;

  // bool should_delete = false;
  int entries_to_delete_from_back = 0;

  // loop through all available log entries
  for (int i = 0; i < QLOCX_LOG_MAX_ENTRIES; i++)
  {
    entries_to_delete_from_back++;
    // size_t page_index = QLOCX_LOG_FLASH_PAGE_INDEX;
    size_t byte_index = QLOCX_LOG_ENTRY_SIZE * i;
    // if the date of the entry is less than or equal to the date
    // we're searching for, we skip it
    if (qlocx_flash_cmp(QLOCX_LOG_FLASH_PAGE_INDEX, byte_index, hash, sizeof(hash)) == 0) {
      NRF_LOG_INFO("Hittade inte hashen inuti flash index %d, fortsätter", i);
      continue;
    } else {
      NRF_LOG_INFO("Hittade hashen! %d", hash);
    }
    // if the entry is empty, we skip it
    // if (qlocx_flash_is_empty(QLOCX_LOG_FLASH_PAGE_INDEX, byte_index, sizeof(hash))) continue;
    // save entry
    // qlocx_flash_get(QLOCX_LOG_FLASH_PAGE_INDEX, byte_index, buffer + saved_entries * QLOCX_LOG_ENTRY_SIZE, QLOCX_LOG_ENTRY_SIZE);
  }

  // qlocx_flash_erase_page(QLOCX_LOG_FLASH_PAGE_INDEX);
  // qlocx_flash_set(
  //   QLOCX_LOG_FLASH_PAGE_INDEX,
  //   0,
  //   buffer,
  //   saved_entries * QLOCX_LOG_ENTRY_SIZE);

  // qlocx_log_num_entries = saved_entries;
}

//TODO: radera från flashminnet också
void qlocx_log_clear_all()
{
  qlocx_flash_erase_page(QLOCX_LOG_FLASH_PAGE_INDEX);
}

size_t qlocx_log_size()
{
  return qlocx_log_num_entries * QLOCX_LOG_ENTRY_SIZE;
}

void qlocx_log_cpy(void* dest)
{
  qlocx_flash_get(QLOCX_LOG_FLASH_PAGE_INDEX, 0, dest, qlocx_log_size());
}

