/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "qlocx_crypto.h"
#include <sodium.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "nrf_soc.h"

#include "nrf_log.h"

static uint32_t qlocx_crypto_random_word(void)
{
  uint32_t n = 0;
  sd_rand_application_vector_get((uint8_t*) &n, 4);
  return n;
}

static void qlocx_crypto_random_buf(void* const buf, const size_t size)
{
  sd_rand_application_vector_get(buf, size);
}

/**
 * Initialize the cryptographic utilities.
 * This function should be called before any
 * member of this module is used.
 */
uint32_t qlocx_crypto_init()
{
  NRF_LOG_INFO("Crypto initializing.");
  static randombytes_implementation implementation;
  implementation.random = qlocx_crypto_random_word;
  implementation.buf = qlocx_crypto_random_buf;
  randombytes_set_implementation(&implementation);
  return sodium_init();
}

/**
 * Generate a key to to used for symmetric encryption.
 *
 * @param key the location to write the key to
 */
void qlocx_crypto_generate_symmetric_key(uint8_t* key)
{
  crypto_secretbox_keygen(key);
}

/**
 * Generate a keypair to to used for asymmetric encryption.
 *
 * @param public_key the location to write the public key to
 * @param private_key the location to write the private key to
 */
void qlocx_crypto_generate_asymmetric_keypair(uint8_t *public_key, uint8_t *private_key)
{
  crypto_box_keypair(public_key, private_key);
}

/**
 * Encrypt a message in place via symmetric encryption.
 *
 * @param message the plaintext to be encrypted
 * @param length the length of the plaintext
 * @param key the key to encrypt the message with
 * @return the length of the message after encryption or 0 if encryption failed
 */
uint16_t qlocx_crypto_encrypt_symmetric_in_place(uint8_t *message, uint16_t length, const uint8_t* key)
{

  // make place for nonce
  memmove(
    message + crypto_secretbox_NONCEBYTES,
    message,
    length);

  // write nonce
  randombytes_buf(message, crypto_secretbox_NONCEBYTES);

  // encrypt message
  bool error = crypto_secretbox_easy(
    message + crypto_secretbox_NONCEBYTES, // message destination
    message + crypto_secretbox_NONCEBYTES, // cipher source
    length,                                // cipher length
    message,                               // nonce source
    key                                    // key
  );

  if (error) return 0;

  return length + crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES;

}

/**
 * Decrypt a message in place via symmetric decryption.
 *
 * @param message the cipher to be decrypted
 * @param length the length of the cipher
 * @param key the key to use for decryption
 * @return the length of the message after decryption or 0 if decryption failed
 */
uint16_t qlocx_crypto_decrypt_symmetric_in_place(uint8_t *message, uint16_t length, const uint8_t *key)
{

  // decrypt cipher
  bool error = crypto_secretbox_open_easy(
    message + crypto_secretbox_NONCEBYTES, // message destination
    message + crypto_secretbox_NONCEBYTES, // cipher source
    length - crypto_secretbox_NONCEBYTES,  // cipher length
    message,                               // nonce source
    key                                    // key
  );

  if (error) return 0;

  // move to beginning
  memmove(
    message,
    message + crypto_secretbox_NONCEBYTES,
    length - crypto_secretbox_NONCEBYTES - crypto_secretbox_MACBYTES);

  return length - crypto_secretbox_NONCEBYTES - crypto_secretbox_MACBYTES;

}

/**
 * Encrypt a message in place via asymmetric encryption.
 *
 * @return the length of the message after encryption or 0 if encryption failed
 */
uint16_t qlocx_crypto_encrypt_asymmetric_in_place(uint8_t *message, uint16_t length,
    const uint8_t *recipient_public_key, const uint8_t *sender_private_key)
{

  uint8_t *nonce = message;
  uint8_t *data = message + crypto_box_NONCEBYTES;
  uint8_t *cipher = message + crypto_box_NONCEBYTES;
  uint16_t data_length = length;
  uint16_t cipher_length = data_length + crypto_box_MACBYTES;

  // make place for nonce
  memmove(
    data,
    message,
    data_length);

  // write nonce
  randombytes_buf(nonce, crypto_box_NONCEBYTES);

  int error = crypto_box_easy(
    cipher,
    data,
    data_length,
    nonce,
    recipient_public_key,
    sender_private_key);

  if (error) return 0;

  return cipher_length + crypto_box_NONCEBYTES;

}

/**
 * Decrypt a message in place via asymmetric encryption.
 *
 * @return the length of the message after decryption or 0 if decryption failed
 */
uint16_t qlocx_crypto_decrypt_asymmetric_in_place(uint8_t *data, uint16_t data_length,
    const uint8_t *sender_public_key, const uint8_t *recipient_private_key)
{

  uint8_t *nonce = data;
  uint8_t *cipher = data + crypto_box_NONCEBYTES;
  uint8_t *message = data + crypto_box_NONCEBYTES;
  uint16_t cipher_length = data_length - crypto_box_NONCEBYTES;
  uint16_t message_length = cipher_length - crypto_box_MACBYTES;

  int error = crypto_box_open_easy(
    message,
    cipher,
    cipher_length,
    nonce,
    sender_public_key,
    recipient_private_key);

  if (error) return 0;

  // adjust message
  memmove(data, message, message_length);

  return message_length;

}

/**
 * Verify a message and a corresponding hash via hmac-sha256.
 *
 * @param message the message to be verified
 * @param length the length of the message
 * @param hash the hash digest to be verified
 * @param key the key to use for verification
 * @return true on success and false on failure
 */
bool qlocx_crypto_verify_hmacsha256(const uint8_t *message, uint16_t length, const uint8_t *hash, const uint8_t *key)
{

  return crypto_auth_hmacsha256_verify(
    hash,
    message,
    length,
    key
  ) == 0;

}
