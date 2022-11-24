
/* This file was automatically generated by nrfutil on 2018-06-12 (YY-MM-DD) at 15:30:40 */

#include "sdk_config.h"
#include "stdint.h"
#include "compiler_abstraction.h"

#if NRF_CRYPTO_BACKEND_OBERON_ENABLED
/* Oberon backend is changing endianness thus public key must be kept in RAM. */
#define _PK_CONST
#else
#define _PK_CONST const
#endif


/** @brief Public key used to verify DFU images */
__ALIGN(4) _PK_CONST uint8_t pk[64] =
{
    0xbb, 0xa2, 0x2f, 0x2b, 0x9f, 0x76, 0x08, 0x12, 0xcd, 0x2e, 0x64, 0x0b, 0x7b, 0x59, 0x3c, 0x59, 0x52, 0xd4, 0x93, 0xd1, 0xb9, 0xd5, 0x68, 0x21, 0x8e, 0x7e, 0x3b, 0xc5, 0x53, 0xba, 0x8e, 0x22, 
    0x15, 0x76, 0x88, 0xbd, 0x8a, 0xd6, 0xcb, 0x0e, 0x9c, 0x28, 0xfe, 0xf1, 0x91, 0xca, 0x25, 0xca, 0x45, 0xd1, 0xe4, 0xce, 0x65, 0xe3, 0x5a, 0xb7, 0x58, 0xd3, 0x19, 0xb7, 0x56, 0x26, 0xcc, 0xf6
};