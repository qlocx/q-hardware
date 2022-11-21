/*
 * Copyright (c) 2017-2018, Qlocx AB
 *
 * All rights reserved.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_function.h"
#include "nrf_svci_async_handler.h"

#include "nrf_delay.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "peer_manager.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_state.h"
#include "ble_dfu.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "fds.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_bootloader_info.h"

#include "qlocx_api.h"
#include "qlocx_config.h"
#include "qlocx_crypto.h"
#include "qlocx_date.h"
#include "qlocx_flash.h"
#include "qlocx_i2c.h"
#include "qlocx_log.h"
#include "qlocx_ports.h"
#include "qlocx_power_meter.h"
#include "qlocx_rtc.h"
#include "qlocx_ble_service.h"

#define MANUFACTURER_NAME               "Qlocx"

/* the advertising interval (in units of 0.625 ms) */
#define APP_ADV_INTERVAL                150

/* the advertising duration (180 seconds) in units of 10 milliseconds. */
#define APP_ADV_DURATION                0

#define APP_BLE_OBSERVER_PRIO           3
#define APP_BLE_CONN_CFG_TAG            1

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)
#define SLAVE_LATENCY                   0
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)

/* time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)

/* time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)

/* number of attempts before giving up the connection parameter negotiation. */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3

#define SEC_PARAM_BOND                  1
#define SEC_PARAM_MITM                  0
#define SEC_PARAM_LESC                  0
#define SEC_PARAM_KEYPRESS              0
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE
#define SEC_PARAM_OOB                   0
#define SEC_PARAM_MIN_KEY_SIZE          7
#define SEC_PARAM_MAX_KEY_SIZE          16

#define DEAD_BEEF                       0xDEADBEEF

NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);
QLOCX_BLE_DEF(m_qlocx, NRF_SDH_BLE_TOTAL_LINK_COUNT);
BLE_ADVERTISING_DEF(m_advertising);

/* handle of the current connection. */
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

/* forward declaration of advertising start function */
static void qlocx_main_advertising_start(bool erase_bonds);

/* UUIDs for service(s) used in application. */
static ble_uuid_t m_adv_uuids[] = {
  {QLOCX_BLE_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};

uint8_t device_name[QLOCX_CONFIG_DEVICE_NAME_SIZE] = {0};

/**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 *          untill the function returns true. When the function returns true, it means that the
 *          app is ready to reset to DFU mode.
 *
 * @param[in]   event   Power manager event.
 *
 * @retval  True if shutdown is allowed by this power manager handler, otherwise false.
 */
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
  switch (event)
  {
    case NRF_PWR_MGMT_EVT_PREPARE_DFU:
      NRF_LOG_INFO("Power management wants to reset to DFU mode.");
      // YOUR_JOB: Get ready to reset into DFU mode
      //
      // If you aren't finished with any ongoing tasks, return "false" to
      // signal to the system that reset is impossible at this stage.
      //
      // Here is an example using a variable to delay resetting the device.
      //
      // if (!m_ready_for_reset)
      // {
      //      return false;
      // }
      // else
      //{
      //
      //    // Device ready to enter
      //    uint32_t err_code;
      //    err_code = sd_softdevice_disable();
      //    APP_ERROR_CHECK(err_code);
      //    err_code = app_timer_stop_all();
      //    APP_ERROR_CHECK(err_code);
      //}
      break;

    default:
      // YOUR_JOB: Implement any of the other events available from the power management module:
      //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
      //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
      //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
      return true;
  }

  NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
  return true;
}

NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);

static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
  if (state == NRF_SDH_EVT_STATE_DISABLED)
  {
    // Softdevice was disabled before going into reset. Inform bootloader to skip CRC on next boot.
    nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

    //Go to system off.
    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
  }
}

/* nrf_sdh state observer. */
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
  .handler = buttonless_dfu_sdh_state_observer,
};

// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
  switch (event)
  {
    case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
      NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
      // YOUR_JOB: Disconnect all bonded devices that currently are connected.
      //           This is required to receive a service changed indication
      //           on bootup after a successful (or aborted) Device Firmware Update.
      break;

    case BLE_DFU_EVT_BOOTLOADER_ENTER:
      // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
      //           by delaying reset by reporting false in app_shutdown_handler
      NRF_LOG_INFO("Device will enter bootloader mode.");
      break;

    case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
      NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
      APP_ERROR_CHECK(false); // reset device
      break;

    case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
      NRF_LOG_ERROR("Request to send a response to client failed.");
      APP_ERROR_CHECK(false); // reset device
      break;

    default:
      NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
      break;
  }
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
  ret_code_t err_code;

  switch (p_evt->evt_id)
  {
    case PM_EVT_BONDED_PEER_CONNECTED:
    {
      NRF_LOG_INFO("Connected to a previously bonded device.");
    } break;

    case PM_EVT_CONN_SEC_SUCCEEDED:
    {
      NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                   ble_conn_state_role(p_evt->conn_handle),
                   p_evt->conn_handle,
                   p_evt->params.conn_sec_succeeded.procedure);
    } break;

    case PM_EVT_CONN_SEC_FAILED:
    {
    } break;

    case PM_EVT_CONN_SEC_CONFIG_REQ:
    {
      // Reject pairing request from an already bonded peer.
      pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
      pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
    } break;

    case PM_EVT_STORAGE_FULL:
    {
      // Run garbage collection on the flash.
      err_code = fds_gc();
      if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
      {
        // Retry.
      }
      else
      {
        APP_ERROR_CHECK(err_code);
      }
    } break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED:
    {
      qlocx_main_advertising_start(false);
    } break;

    case PM_EVT_PEER_DATA_UPDATE_FAILED:
    {
      // Assert.
      APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
    } break;

    case PM_EVT_PEER_DELETE_FAILED:
    {
      // Assert.
      APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
    } break;

    case PM_EVT_PEERS_DELETE_FAILED:
    {
      // Assert.
      APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
    } break;

    case PM_EVT_ERROR_UNEXPECTED:
    {
      // Assert.
      APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
    } break;

    case PM_EVT_CONN_SEC_START:
    case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
    case PM_EVT_PEER_DELETE_SUCCEEDED:
    case PM_EVT_LOCAL_DB_CACHE_APPLIED:
    case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
      // This can happen when the local DB has changed.
    case PM_EVT_SERVICE_CHANGED_IND_SENT:
    case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
    default:
      break;
  }
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void qlocx_main_timers_init(void)
{
  uint32_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void qlocx_main_gap_params_init(void)
{
  uint32_t                err_code;
  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  err_code = sd_ble_gap_device_name_set(&sec_mode,
                                      (const uint8_t*) device_name,
                                      strlen((char*) device_name));
  APP_ERROR_CHECK(err_code);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing services that will be used by the application.
 */
static void qlocx_main_services_init(void)
{
  uint32_t                  err_code;
  nrf_ble_qwr_init_t        qwr_init    = {0};
  ble_dfu_buttonless_init_t dfus_init   = {0};
  qlocx_ble_init_t          qlocx_init  = {0};

  // initialize qlocx service
  err_code = qlocx_ble_init(&m_qlocx, &qlocx_init);
  APP_ERROR_CHECK(err_code);

  // Initialize Queued Write Module.
  qwr_init.error_handler = nrf_qwr_error_handler;
  err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
  APP_ERROR_CHECK(err_code);

  // Initialize the async SVCI interface to bootloader.
  err_code = ble_dfu_buttonless_async_svci_init();
  APP_ERROR_CHECK(err_code);
  dfus_init.evt_handler = ble_dfu_evt_handler;

  err_code = ble_dfu_buttonless_init(&dfus_init);
  APP_ERROR_CHECK(err_code);

}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
  uint32_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
  {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void qlocx_main_conn_params_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void qlocx_main_conn_params_init(void)
{
  uint32_t               err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail             = false;
  cp_init.evt_handler                    = on_conn_params_evt;
  cp_init.error_handler                  = qlocx_main_conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void qlocx_main_app_timers_start(void)
{
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void qlocx_main_sleep_mode_enter(void)
{
  uint32_t err_code;
  err_code = nrf_sdh_disable_request();
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
  switch (ble_adv_evt)
  {
    case BLE_ADV_EVT_FAST:
      break;

    case BLE_ADV_EVT_IDLE:
      qlocx_main_sleep_mode_enter();
      break;

    default:
      break;
  }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
  uint32_t err_code = NRF_SUCCESS;

  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_DISCONNECTED:
      break;

    case BLE_GAP_EVT_CONNECTED:
      m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
      err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
      NRF_LOG_DEBUG("PHY update request.");
      ble_gap_phys_t const phys =
      {
        .rx_phys = BLE_GAP_PHY_AUTO,
        .tx_phys = BLE_GAP_PHY_AUTO,
      };
      err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
      APP_ERROR_CHECK(err_code);
    } break;

    case BLE_GATTC_EVT_TIMEOUT:
      NRF_LOG_DEBUG("GATT Client Timeout.");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_GATTS_EVT_TIMEOUT:
      NRF_LOG_DEBUG("GATT Server Timeout.");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
      break;

    default:
      // No implementation needed.
      break;
  }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void qlocx_main_ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Add event listeners.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for the Peer Manager initialization.
 */
static void qlocx_main_peer_manager_init()
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/** @brief Clear bonding information from persistent storage.
 */
static void qlocx_main_delete_bonds(void)
{
    ret_code_t err_code;
    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void qlocx_main_advertising_init(void)
{
  uint32_t               err_code;
  ble_advertising_init_t init;

  memset(&init, 0, sizeof(init));

  init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
  init.advdata.include_appearance      = false;
  init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
  init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

  init.config.ble_adv_fast_enabled  = true;
  init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
  init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

  init.evt_handler = on_adv_evt;

  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for the Power manager.
 */
static void qlocx_main_log_init(void)
{
  uint32_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief   Function for initializing the GATT module.
 * @details The GATT module handles ATT_MTU and Data Length update procedures automatically.
 */
static void qlocx_main_gatt_init(void)
{
  ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting advertising.
 */
static void qlocx_main_advertising_start(bool erase_bonds)
{
  if (erase_bonds)
  {
    qlocx_main_delete_bonds();
  }
  else
  {
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

  }
}

static void qlocx_main_power_management_init(void)
{
  uint32_t err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for application main entry.
 */
int main(void)
{

    NRF_LOG_DEBUG("Inuti main");

    bool erase_bonds = false;

    qlocx_main_log_init();

    qlocx_flash_init();
    qlocx_config_init();
    qlocx_config_get_device_name(device_name);

    qlocx_main_timers_init();
    qlocx_main_power_management_init();
    qlocx_main_ble_stack_init();
    qlocx_main_peer_manager_init();
    qlocx_main_gap_params_init();
    qlocx_main_gatt_init();
    qlocx_main_services_init();
    qlocx_main_advertising_init();
    qlocx_main_conn_params_init();

    qlocx_crypto_init();
    qlocx_date_init();
    qlocx_log_init();
    qlocx_i2c_init();
#ifdef QLOCX_RTC_ENABLED
    qlocx_rtc_init();
#endif
#ifdef QLOCX_POWER_METER_ENABLED
    qlocx_power_meter_init();
#endif
    qlocx_api_init();
    qlocx_ports_init();

    qlocx_main_app_timers_start();
    qlocx_main_advertising_start(erase_bonds);

    // Enter main loop.
    for (;;)
    {
      nrf_pwr_mgmt_run();
    }
}

/**
 * @}
 */
