/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include "sdk_common.h"
#include "ble.h"
#include "qlocx_ble_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "app_timer.h"
#include "qlocx_crypto.h"
#include "qlocx_config.h"
#include "qlocx_api.h"
#include "qlocx_log.h"

#include "nrf_log.h"

APP_TIMER_DEF(qlocx_ble_termination_timer);

static app_timer_id_t ble_service_timer;

static void qlocx_ble_terminate_connection(qlocx_ble_t* ctx)
{

  app_timer_stop(qlocx_ble_termination_timer);

  app_timer_stop(ble_service_timer);
  /**
   * If conn_handle is invalid, we have already disconnected.
   */
  NRF_LOG_INFO("Terminating before handle check.\n");
  if (ctx->conn_handle == BLE_CONN_HANDLE_INVALID) return;

  NRF_LOG_INFO("Terminating connection.\n");

  uint32_t err_code = sd_ble_gap_disconnect(
    ctx->conn_handle,
    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);

  APP_ERROR_CHECK(err_code);

}

/**
 * This callback is called if the remote has timed out.
 */
static void qlocx_ble_termination_timer_handler(void* context)
{
  NRF_LOG_INFO("Remote timed out.\n");
  qlocx_ble_terminate_connection((qlocx_ble_t*)context);
}

static void qlocx_ble_flush_tx_buffer(qlocx_ble_t* ctx)
{
  uint32_t err_code;

  if (!ctx->tx_length) return;

  // create a chunk from tx buffer of the largest possible transmission size
  uint8_t chunk_length = MIN(
    ctx->tx_length,
    QLOCX_BLE_MAX_DATA_LEN
  );

  // transmit chunk
  // NRF_LOG_INFO("Sending %d/%d.\n", chunk_length, ctx->tx_length);
  err_code = qlocx_ble_data_send(ctx, ctx->tx_buffer, chunk_length);

  // terminate connection if transmission failed
  if (err_code != NRF_SUCCESS)
  {
    qlocx_ble_terminate_connection(ctx);
    return;
  }

  // adjust tx buffer
  memmove(ctx->tx_buffer, ctx->tx_buffer + chunk_length, ctx->tx_length - chunk_length);
  ctx->tx_length = ctx->tx_length - chunk_length;

  // send empty message indicating that we have no more data to send
  if (ctx->tx_length == 0)
  {
    // NRF_LOG_INFO("Färdigt, skickar null-byte");
    qlocx_ble_data_send(ctx, ctx->tx_buffer, 0);
  }

}

static void data_handler(qlocx_ble_t* context, const uint8_t* data, size_t data_length) {

  app_timer_stop(qlocx_ble_termination_timer);
  app_timer_start(qlocx_ble_termination_timer, APP_TIMER_TICKS(30000), context);
  app_timer_start(ble_service_timer, 5 * 32768, NULL);

  // terminate connection if the user sends too much data
  if (context->rx_length + data_length > sizeof(context->rx_buffer)) {
    NRF_LOG_INFO("Terminating request because of too much data.");
    qlocx_ble_terminate_connection(context);
    return;
  }

  // copy the packet data onto request buffer
  memcpy(context->rx_buffer + context->rx_length, data, data_length);
  context->rx_length += data_length;

  // if the packet wasn't empty, we expect another one
  if (data_length) return;

  if (!context->encryption_configured) {

    NRF_LOG_INFO("Responding to unencrypted request.");

    // terminate connection if remote didn't send a public key
    if (context->rx_length != QLOCX_CRYPTO_ASYMMETRIC_PUBLIC_KEY_SIZE) {
      NRF_LOG_INFO("Terminating request, remote didn't send a public key.");
      qlocx_ble_terminate_connection(context);
      return;
    }

    // save remote public key
    NRF_LOG_INFO("Saving remote public key.");
    memcpy(context->remote_public_key, context->rx_buffer, context->rx_length);

    // generate a session key to be used for the rest of the communication
    NRF_LOG_INFO("Generating session key.");
    qlocx_crypto_generate_symmetric_key(context->session_key);
    NRF_LOG_INFO("Using session key:");
    NRF_LOG_HEXDUMP_INFO(context->session_key, sizeof(context->session_key));

    // copy the session key onto the tx buffer
    memcpy(context->tx_buffer, context->session_key, sizeof(context->session_key));
    context->tx_length = sizeof(context->session_key);

    // encrypt the session key with the public key of the remote
    uint8_t private_key[QLOCX_CONFIG_PRIVATE_KEY_SIZE] = { 0 };
    qlocx_config_get_private_key(private_key);
    NRF_LOG_INFO("Encrypting session key.\n");
    context->tx_length = qlocx_crypto_encrypt_asymmetric_in_place(
      context->tx_buffer,
      context->tx_length,
      context->remote_public_key,
      private_key);

    // terminate request if encryption failed
    if (!context->tx_length) {
      NRF_LOG_INFO("Terminating request, couldn't encrypt the session key.\n");
      qlocx_ble_terminate_connection(context);
      return;
    }

    context->encryption_configured = true;
    context->rx_length = 0;

    // send response
    qlocx_ble_flush_tx_buffer(context);

  }
  else {

    // terminate connection if rx buffer is too short
    if (context->rx_length < 72) {
      NRF_LOG_INFO("Invalid request length %d.\n", context->rx_length);
      qlocx_ble_terminate_connection(context);
      return;
    }

    // decrypt the request
    context->rx_length = qlocx_crypto_decrypt_symmetric_in_place(
      context->rx_buffer,
      context->rx_length,
      context->session_key);

    // terminate connection if decryption failed
    if (!context->rx_length) {
      NRF_LOG_INFO("Decryption failed.\n");
      qlocx_ble_terminate_connection(context);
      return;
    }

    uint8_t private_key[QLOCX_CONFIG_PRIVATE_KEY_SIZE] = { 0 };
    qlocx_config_get_private_key(private_key);
    // verify hash
    bool success = qlocx_crypto_verify_hmacsha256(
      context->rx_buffer + QLOCX_CRYPTO_HMACSHA256_HASH_SIZE,
      context->rx_length - QLOCX_CRYPTO_HMACSHA256_HASH_SIZE,
      context->rx_buffer,
      private_key);

    // terminate connection if hash is invalid
    if (!success) {
      NRF_LOG_INFO("Invalid request hash.\n");
      qlocx_ble_terminate_connection(context);
      return;
    }

    // save the hash in a temporary buffer
    uint8_t hash[QLOCX_CRYPTO_HMACSHA256_HASH_SIZE] = { 0 };
    memcpy(hash, context->rx_buffer, sizeof(hash));
    memmove(context->rx_buffer, context->rx_buffer + sizeof(hash), context->rx_length - sizeof(hash));
    context->rx_length -= sizeof(hash);

    // handle request
    switch (context->rx_buffer[0]) {
#ifdef QLOCX_POWER_METER_ENABLED
    case QLOCX_API_GET_BATTERY_STATUS:
      success = qlocx_api_get_battery_status(context);
      break;
#endif
    case QLOCX_API_OPEN_PORT:
      success = qlocx_api_open_port(context);
      break;
    case QLOCX_API_GET_PORT_STATUS:
      success = qlocx_api_get_port_status(context);
      break;
    case QLOCX_API_SET_SENSE_CONFIG:
      success = qlocx_api_set_sense_config(context);
      break;
    case QLOCX_API_PURGE_LOGS_UNTIL_DATE:
      success = qlocx_api_purge_logs_until_date(context);
      break;
    case QLOCX_API_PURGE_LOGS_UNTIL_HASH:
      success = qlocx_api_purge_logs_until_hash(context);
      break;
    case QLOCX_API_GET_LOGS:
      success = qlocx_api_get_logs(context);
      break;
    case QLOCX_API_GET_SOFTWARE_VERSION:
      success = qlocx_api_get_software_version(context);
      break;
#ifdef QLOCX_RTC_ENABLED
    case QLOCX_API_GET_DATE:
      success = qlocx_api_get_date(context);
      break;
#endif
    default:
      success = false;
      break;
    }

    if (!success) {
      NRF_LOG_INFO("Error in API.");
      qlocx_ble_terminate_connection(context);
      return;
    }

    // add the hash to the log
    qlocx_log_add_entry(hash);

    // make place for hash in tx_buffer
    memmove(context->tx_buffer + QLOCX_CRYPTO_HMACSHA256_HASH_SIZE, context->tx_buffer, context->tx_length);
    context->tx_length += QLOCX_CRYPTO_HMACSHA256_HASH_SIZE;

    // copy hash onto the tx_buffer
    memcpy(context->tx_buffer, hash,
      QLOCX_CRYPTO_HMACSHA256_HASH_SIZE);

    // empty rx buffer
    context->rx_length = 0;

    // encrypt response
    context->tx_length = qlocx_crypto_encrypt_symmetric_in_place(
      context->tx_buffer,
      context->tx_length,
      context->session_key);

    // terminate connection if encryption failed
    if (!context->tx_length) {
      NRF_LOG_INFO("Encryption failed.\n");
      qlocx_ble_terminate_connection(context);
      return;
    }

    // send response
    qlocx_ble_flush_tx_buffer(context);

  }

}

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void qlocx_ble_on_connect(qlocx_ble_t* ctx, ble_evt_t const* p_ble_evt)
{
  ret_code_t                err_code;
  qlocx_ble_evt_t           evt;
  ble_gatts_value_t         gatts_val;
  uint8_t                   cccd_value[2];
  ble_nus_client_context_t* p_client = NULL;

  err_code = blcm_link_ctx_get(ctx->p_link_ctx_storage,
    p_ble_evt->evt.gap_evt.conn_handle,
    (void*)&p_client);
  if (err_code != NRF_SUCCESS)
  {
    NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
      p_ble_evt->evt.gap_evt.conn_handle);
  }

  /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
  memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
  gatts_val.p_value = cccd_value;
  gatts_val.len = sizeof(cccd_value);
  gatts_val.offset = 0;

  err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
    ctx->rx_handles.cccd_handle,
    &gatts_val);

  if ((err_code == NRF_SUCCESS) &&
    ble_srv_is_notification_enabled(gatts_val.p_value))
  {
    if (p_client != NULL)
    {
      p_client->is_notification_enabled = true;
    }

    memset(&evt, 0, sizeof(qlocx_ble_evt_t));
    evt.type = QLOCX_BLE_EVT_COMM_STARTED;
    evt.p_nus = ctx;
    evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    evt.p_link_ctx = p_client;

  }

  NRF_LOG_INFO("New connection.");
  ctx->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
  ctx->encryption_configured = false;
  ctx->rx_length = 0;
  ctx->tx_length = 0;

  NRF_LOG_INFO("Remote timeout in thirty seconds.");
  app_timer_start(qlocx_ble_termination_timer, APP_TIMER_TICKS(30000), ctx);
  app_timer_start(ble_service_timer, APP_TIMER_TICKS(5000), ctx);

}

/**
 * Callback for when a connection is terminated.
 */
static void qlocx_ble_on_disconnect(qlocx_ble_t* context, const ble_evt_t* event)
{
  NRF_LOG_INFO("Disconnected.\n");
  context->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void qlocx_ble_on_write(qlocx_ble_t* ctx, ble_evt_t const* p_ble_evt)
{
  ret_code_t                    err_code;
  qlocx_ble_evt_t               evt;
  ble_nus_client_context_t* p_client;
  ble_gatts_evt_write_t const* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

  err_code = blcm_link_ctx_get(ctx->p_link_ctx_storage,
    p_ble_evt->evt.gatts_evt.conn_handle,
    (void*)&p_client);
  if (err_code != NRF_SUCCESS)
  {
    NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
      p_ble_evt->evt.gatts_evt.conn_handle);
  }

  memset(&evt, 0, sizeof(qlocx_ble_evt_t));
  evt.p_nus = ctx;
  evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
  evt.p_link_ctx = p_client;

  // change of notification settings
  if ((p_evt_write->handle == ctx->tx_handles.cccd_handle) &&
    (p_evt_write->len == 2))
  {
    if (p_client != NULL)
    {
      if (ble_srv_is_notification_enabled(p_evt_write->data))
      {
        p_client->is_notification_enabled = true;
        evt.type = QLOCX_BLE_EVT_COMM_STARTED;
      }
      else
      {
        p_client->is_notification_enabled = false;
        evt.type = QLOCX_BLE_EVT_COMM_STOPPED;
      }
    }
  }
  // normal write to the rx handle
  else if (p_evt_write->handle == ctx->rx_handles.value_handle)
  {
    evt.type = QLOCX_BLE_EVT_RX_DATA;
    evt.params.rx_data.p_data = p_evt_write->data;
    evt.params.rx_data.length = p_evt_write->len;

    data_handler(
      ctx,
      p_evt_write->data,
      p_evt_write->len);
  }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void qlocx_ble_on_hvx_tx_complete(qlocx_ble_t* ctx, ble_evt_t const* p_ble_evt)
{
  ret_code_t                err_code;
  qlocx_ble_evt_t           evt;
  ble_nus_client_context_t* p_client;

  err_code = blcm_link_ctx_get(ctx->p_link_ctx_storage,
    p_ble_evt->evt.gatts_evt.conn_handle,
    (void*)&p_client);
  if (err_code != NRF_SUCCESS)
  {
    NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
      p_ble_evt->evt.gatts_evt.conn_handle);
    return;
  }

  if (p_client->is_notification_enabled)
  {
    memset(&evt, 0, sizeof(qlocx_ble_evt_t));
    evt.type = QLOCX_BLE_EVT_TX_RDY;
    evt.p_nus = ctx;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx = p_client;

  }
}


/**@brief Function for adding TX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t qlocx_ble_tx_char_add(qlocx_ble_t* ctx, qlocx_ble_init_t const* p_nus_init)
{
  ble_gatts_char_md_t char_md;
  ble_gatts_attr_md_t cccd_md;
  ble_gatts_attr_t    attr_char_value;
  ble_uuid_t          ble_uuid;
  ble_gatts_attr_md_t attr_md;

  memset(&cccd_md, 0, sizeof(cccd_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

  cccd_md.vloc = BLE_GATTS_VLOC_STACK;

  memset(&char_md, 0, sizeof(char_md));

  char_md.char_props.notify = 1;
  char_md.p_char_user_desc = NULL;
  char_md.p_char_pf = NULL;
  char_md.p_user_desc_md = NULL;
  char_md.p_cccd_md = &cccd_md;
  char_md.p_sccd_md = NULL;

  ble_uuid.type = ctx->uuid_type;
  ble_uuid.uuid = QLOCX_BLE_SERVICE_TX_CHARACTERISTIC_UUID;

  memset(&attr_md, 0, sizeof(attr_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

  attr_md.vloc = BLE_GATTS_VLOC_STACK;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen = 1;

  memset(&attr_char_value, 0, sizeof(attr_char_value));

  attr_char_value.p_uuid = &ble_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len = sizeof(uint8_t);
  attr_char_value.init_offs = 0;
  attr_char_value.max_len = QLOCX_BLE_MAX_DATA_LEN;

  return sd_ble_gatts_characteristic_add(ctx->service_handle,
    &char_md,
    &attr_char_value,
    &ctx->tx_handles);
}


/**@brief Function for adding RX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t qlocx_ble_rx_char_add(qlocx_ble_t* ctx, const qlocx_ble_init_t* p_nus_init)
{
  ble_gatts_char_md_t char_md;
  ble_gatts_attr_t    attr_char_value;
  ble_uuid_t          ble_uuid;
  ble_gatts_attr_md_t attr_md;

  memset(&char_md, 0, sizeof(char_md));

  char_md.char_props.write = 1;
  char_md.char_props.write_wo_resp = 1;
  char_md.p_char_user_desc = NULL;
  char_md.p_char_pf = NULL;
  char_md.p_user_desc_md = NULL;
  char_md.p_cccd_md = NULL;
  char_md.p_sccd_md = NULL;

  ble_uuid.type = ctx->uuid_type;
  ble_uuid.uuid = QLOCX_BLE_SERVICE_RX_CHARACTERISTIC_UUID;

  memset(&attr_md, 0, sizeof(attr_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

  attr_md.vloc = BLE_GATTS_VLOC_STACK;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen = 1;

  memset(&attr_char_value, 0, sizeof(attr_char_value));

  attr_char_value.p_uuid = &ble_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len = 1;
  attr_char_value.init_offs = 0;
  attr_char_value.max_len = QLOCX_BLE_MAX_DATA_LEN;

  return sd_ble_gatts_characteristic_add(ctx->service_handle,
    &char_md,
    &attr_char_value,
    &ctx->rx_handles);
}


void qlocx_ble_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context)
{
  if ((p_context == NULL) || (p_ble_evt == NULL))
  {
    return;
  }

  qlocx_ble_t* ctx = (qlocx_ble_t*)p_context;

  switch (p_ble_evt->header.evt_id)
  {
  case BLE_GAP_EVT_CONNECTED:
    qlocx_ble_on_connect(ctx, p_ble_evt);
    break;

  case BLE_GAP_EVT_DISCONNECTED:
    qlocx_ble_on_disconnect(ctx, p_ble_evt);
    break;

  case BLE_GATTS_EVT_WRITE:
    qlocx_ble_on_write(ctx, p_ble_evt);
    break;

  case BLE_GATTS_EVT_HVN_TX_COMPLETE:
    qlocx_ble_flush_tx_buffer(ctx);
    qlocx_ble_on_hvx_tx_complete(ctx, p_ble_evt);
    break;

  default:
    // No implementation needed.
    break;
  }
}


uint32_t qlocx_ble_init(qlocx_ble_t* ctx, qlocx_ble_init_t const* p_nus_init)
{
  NRF_LOG_INFO("BLE initializing.");
  ret_code_t    err_code;
  ble_uuid_t    ble_uuid;
  ble_uuid128_t nus_base_uuid = QLOCX_BLE_BASE_UUID;

  VERIFY_PARAM_NOT_NULL(ctx);
  VERIFY_PARAM_NOT_NULL(p_nus_init);

  /**@snippet [Adding proprietary Service to the SoftDevice] */
  // Add a custom base UUID.
  err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &ctx->uuid_type);
  APP_ERROR_CHECK(err_code);

  ble_uuid.type = ctx->uuid_type;
  ble_uuid.uuid = QLOCX_BLE_SERVICE_UUID;

  // Add the service.
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
    &ble_uuid,
    &ctx->service_handle);
  /**@snippet [Adding proprietary Service to the SoftDevice] */
  APP_ERROR_CHECK(err_code);

  // Add the RX Characteristic.
  err_code = qlocx_ble_rx_char_add(ctx, p_nus_init);
  APP_ERROR_CHECK(err_code);

  // Add the TX Characteristic.
  err_code = qlocx_ble_tx_char_add(ctx, p_nus_init);
  APP_ERROR_CHECK(err_code);

  app_timer_create(&ble_service_timer, APP_TIMER_MODE_REPEATED, qlocx_ble_termination_timer_handler);

  return NRF_SUCCESS;
}


uint32_t qlocx_ble_data_send(qlocx_ble_t* ctx,
  uint8_t* p_data,
  uint16_t p_length)
{
  ret_code_t                 err_code;
  ble_gatts_hvx_params_t     hvx_params;
  ble_nus_client_context_t* p_client;

  VERIFY_PARAM_NOT_NULL(ctx);

  err_code = blcm_link_ctx_get(ctx->p_link_ctx_storage, ctx->conn_handle, (void*)&p_client);
  VERIFY_SUCCESS(err_code);

  if ((ctx->conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
  {
    return NRF_ERROR_NOT_FOUND;
  }

  if (!p_client->is_notification_enabled)
  {
    return NRF_ERROR_INVALID_STATE;
  }

  if (p_length > QLOCX_BLE_MAX_DATA_LEN)
  {
    return NRF_ERROR_INVALID_PARAM;
  }

  memset(&hvx_params, 0, sizeof(hvx_params));

  hvx_params.handle = ctx->tx_handles.value_handle;
  hvx_params.p_data = p_data;
  hvx_params.p_len = &p_length;
  hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

  return sd_ble_gatts_hvx(ctx->conn_handle, &hvx_params);
}


