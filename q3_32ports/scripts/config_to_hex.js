#include "app/include/qlocx_defines.h"

const MemoryMap = require('nrf-intel-hex')
const fs        = require('fs')

const CONFIG_BASE = (QLOCX_FLASH_START_ADDR + QLOCX_FLASH_PAGE_SIZE * QLOCX_CONFIG_FLASH_PAGE_INDEX)

const addresses = {
  SENSE_CONFIG_ADDRESS: (CONFIG_BASE + QLOCX_CONFIG_SENSE_CONFIG_BYTE_INDEX),
  PUBLIC_KEY_ADDRESS:   (CONFIG_BASE + QLOCX_CONFIG_PUBLIC_KEY_BYTE_INDEX),
  PRIVATE_KEY_ADDRESS:  (CONFIG_BASE + QLOCX_CONFIG_PRIVATE_KEY_BYTE_INDEX),
  DEVICE_NAME_ADDRESS:  (CONFIG_BASE + QLOCX_CONFIG_DEVICE_NAME_BYTE_INDEX),
  DATE_ADDRESS:         (CONFIG_BASE + QLOCX_CONFIG_DATE_BYTE_INDEX)
}

let config = JSON.parse(fs.readFileSync('build/config.json').toString())
  
/**
 * Create config as a Intel hex-file.
 */
let map = new MemoryMap()
for (const key of Object.keys(config))
  map.set(parseInt(addresses[key.toUpperCase() + '_ADDRESS']), Buffer.from(config[key], 'hex'))
console.log(map.asHexString())

