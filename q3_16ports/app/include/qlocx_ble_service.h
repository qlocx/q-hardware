/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#ifndef QLOCX_BLE_SERVICE_H
#define QLOCX_BLE_SERVICE_H

#include "ble.h"
#include "ble_link_ctx_manager.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "qlocx_crypto.h"
#include "sdk_config.h"
#include <stdbool.h>
#include <stdint.h>

#define QLOCX_BLE_DEF(_name, _nus_max_clients)                      \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),  \
                             (_nus_max_clients),                  \
                             sizeof(ble_nus_client_context_t));   \
    static qlocx_ble_t _name =                                      \
    {                                                             \
        .uuid_type          = 0x02,                               \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage) \
    };                                                            \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,                           \
                         BLE_NUS_BLE_OBSERVER_PRIO,               \
                         qlocx_ble_on_ble_evt,                      \
                         &_name)

#define QLOCX_BLE_SERVICE_UUID 0x0001

#define QLOCX_BLE_SERVICE_TX_CHARACTERISTIC_UUID 0x0003
#define QLOCX_BLE_SERVICE_RX_CHARACTERISTIC_UUID 0x0002

#define QLOCX_BLE_BASE_UUID {{ \
    0xfc, 0x6b, 0xf1, 0xe1, 0x27, 0x7f, 0xd9, 0x88, \
    0xf5, 0x48, 0xbb, 0xb1, 0x00, 0x00, 0x13, 0x83 \
}}

#define OPCODE_LENGTH        1
#define HANDLE_LENGTH        2

#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define QLOCX_BLE_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define QLOCX_BLE_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif


typedef enum
{
    QLOCX_BLE_EVT_RX_DATA,      /**< Data received. */
    QLOCX_BLE_EVT_TX_RDY,       /**< Service is ready to accept new data to be transmitted. */
    QLOCX_BLE_EVT_COMM_STARTED, /**< Notification has been enabled. */
    QLOCX_BLE_EVT_COMM_STOPPED, /**< Notification has been disabled. */
} qlocx_ble_evt_type_t;

/* Forward declaration of the qlocx_ble_t type. */
typedef struct qlocx_ble_s qlocx_ble_t;
typedef struct
{
    uint8_t const * p_data; /**< A pointer to the buffer with received data. */
    uint16_t        length; /**< Length of received data. */
} qlocx_ble_evt_rx_data_t;

typedef struct
{
    bool is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
} ble_nus_client_context_t;

typedef struct
{
    qlocx_ble_evt_type_t         type;        /**< Event type. */
    qlocx_ble_t                * p_nus;       /**< A pointer to the instance. */
    uint16_t                   conn_handle; /**< Connection handle. */
    ble_nus_client_context_t * p_link_ctx;  /**< A pointer to the link context. */
    union
    {
        qlocx_ble_evt_rx_data_t rx_data; /**< @ref QLOCX_BLE_EVT_RX_DATA event data. */
    } params;
} qlocx_ble_evt_t;

typedef void (* ble_nus_data_handler_t) (qlocx_ble_evt_t * p_evt);

typedef struct
{
    ble_nus_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} qlocx_ble_init_t;

struct qlocx_ble_s
{
	uint8_t                         uuid_type;          /**< UUID type for Nordic UART Service Base UUID. */
	uint16_t                        service_handle;     /**< Handle of Nordic UART Service (as provided by the SoftDevice). */
	ble_gatts_char_handles_t        tx_handles;         /**< Handles related to the TX characteristic (as provided by the SoftDevice). */
	ble_gatts_char_handles_t        rx_handles;         /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
	blcm_link_ctx_storage_t * const p_link_ctx_storage; /**< Pointer to link context storage with handles of all current connections and its context. */
	ble_nus_data_handler_t          data_handler;       /**< Event handler to be called for handling received data. */
	uint16_t conn_handle;
	uint8_t tx_buffer[128];
	uint8_t rx_buffer[128];
	size_t tx_length;
	size_t rx_length;
  /**
   * Boolean indicating whether encryption has been configured for the
   * current connection.
   */
  bool                     encryption_configured;
  /**
   * The session key used for encrypted communication with the remote.
   */
  uint8_t                  session_key[QLOCX_CRYPTO_SYMMETRIC_KEY_SIZE];
  /**
   * The public key of the remote used to encrypt the session key in
   * the key negotiation.
   */
	uint8_t remote_public_key[QLOCX_CRYPTO_ASYMMETRIC_PUBLIC_KEY_SIZE];
};

uint32_t qlocx_ble_init(qlocx_ble_t * p_nus, qlocx_ble_init_t const * p_nus_init);

void qlocx_ble_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

uint32_t qlocx_ble_data_send(qlocx_ble_t*, uint8_t*, uint16_t);

#endif
