#include "app/include/qlocx_defines.h"

const sodium = require('sodium-native')

let config = {
  sense_config: Buffer.alloc(QLOCX_CONFIG_SENSE_CONFIG_SIZE),
  public_key: Buffer.alloc(QLOCX_CONFIG_PUBLIC_KEY_SIZE),
  private_key: Buffer.alloc(QLOCX_CONFIG_PRIVATE_KEY_SIZE),
  device_name: Buffer.alloc(QLOCX_CONFIG_DEVICE_NAME_SIZE),
  date: Buffer.alloc(QLOCX_CONFIG_DATE_SIZE),
}

/**
 * Generate a new keypair.
 */
sodium.crypto_box_keypair(config.public_key, config.private_key)

/**
 * Use the first eight characters of the hex
 * representation of the public key as the device name 
 */
config.device_name.write(config.public_key.toString('hex').slice(0, 8))

/**
 * Use the current date as the date to program
 * onto the device.
 */
Buffer.from([
  new Date().getFullYear() - 2000,
  new Date().getMonth() + 1,
  new Date().getDate(),
  new Date().getHours(),
  new Date().getMinutes(),
  new Date().getSeconds(),
]).copy(config.date)

for (const key of Object.keys(config))
  config[key] = config[key].toString('hex')

console.log(JSON.stringify(config, null, 2))

