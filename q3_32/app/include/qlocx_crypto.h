/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

/**
 * @file qlocx_crypto.h
 *
 * The crypto module contains the cryptographic
 * utilities used by the board.
 */

#ifndef QLOCX_CRYPTO_H
#define QLOCX_CRYPTO_H

#include <stdbool.h>
#include <stdint.h>

/**
 * The length in bytes of a public key for asymmetric
 * encryption.
 */
#define QLOCX_CRYPTO_ASYMMETRIC_PUBLIC_KEY_SIZE  32

/**
 * The length in bytes of a private key for asymmetric
 * encryption.
 */
#define QLOCX_CRYPTO_ASYMMETRIC_PRIVATE_KEY_SIZE 32

/**
 * The length in bytes of a key for symmetric encryption.
 */
#define QLOCX_CRYPTO_SYMMETRIC_KEY_SIZE          32

/**
 * The length in bytes of a HMACSHA256 hash digest.
 */
#define QLOCX_CRYPTO_HMACSHA256_HASH_SIZE        32

bool     qlocx_crypto_verify_hmacsha256(const uint8_t *message, uint16_t length, const uint8_t *hash, const uint8_t *key);
uint16_t qlocx_crypto_encrypt_asymmetric_in_place(uint8_t *message, uint16_t length, const uint8_t *recipient_public_key, const uint8_t *sender_private_key);
uint16_t qlocx_crypto_decrypt_asymmetric_in_place(uint8_t *message, uint16_t length, const uint8_t *sender_public_key, const uint8_t *recipient_private_key);
uint16_t qlocx_crypto_decrypt_symmetric_in_place(uint8_t *message, uint16_t length, const uint8_t* key);
uint16_t qlocx_crypto_encrypt_symmetric_in_place(uint8_t *message, uint16_t length, const uint8_t* key);
void     qlocx_crypto_generate_symmetric_key(uint8_t *key);
void     qlocx_crypto_generate_asymmetric_keypair(uint8_t *public_key, uint8_t *private_key);
uint32_t qlocx_crypto_init(void);

#endif
