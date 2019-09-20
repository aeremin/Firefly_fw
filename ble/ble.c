#include "ble.h"

#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_lbs.h"
#include "ble_srv_common.h"
#include "boards.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define DEVICE_NAME "Nordic_Blinky" /**< Name of device. Will be included in the advertising data. */

#define APP_BLE_OBSERVER_PRIO 3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG 1  /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL 64                                    /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */

#define MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS) /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(200, UNIT_1_25_MS) /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY 0                                    /**< Slave latency. */
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(4000, UNIT_10_MS)   /**< Connection supervisory time-out (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(20000) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3                        /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

BLE_LBS_DEF(m_lbs);       /**< LED Button Service instance. */
NRF_BLE_GATT_DEF(m_gatt); /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);   /**< Context for the Queued Write module.*/

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;           /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];            /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX]; /**< Buffer for storing an encoded scan data. */

static BleCallback gBleCallback = NULL;

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data = {
  .adv_data = {
    .p_data = m_enc_advdata,
    .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX
  },
  .scan_rsp_data = {
    .p_data = m_enc_scan_response_data,
    .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX
  }
};

static void StartAdvertising() {
  APP_ERROR_CHECK(sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG));
}

static void BleEventHandler(ble_evt_t const* p_ble_evt, void* p_context) {
  switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
      NRF_LOG_INFO("Connected");
      m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
      APP_ERROR_CHECK(nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle));
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      NRF_LOG_INFO("Disconnected");
      m_conn_handle = BLE_CONN_HANDLE_INVALID;
      StartAdvertising();
      break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
      // Pairing not supported
      APP_ERROR_CHECK(sd_ble_gap_sec_params_reply(m_conn_handle,
                                                  BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                  NULL,
                                                  NULL));
      break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
      NRF_LOG_DEBUG("PHY update request.");
      ble_gap_phys_t phys;
      phys.rx_phys = BLE_GAP_PHY_AUTO;
      phys.tx_phys = BLE_GAP_PHY_AUTO;
      APP_ERROR_CHECK(sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys));
    } break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
      // No system attributes have been stored.
      APP_ERROR_CHECK(sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0));
      break;

    case BLE_GATTC_EVT_TIMEOUT:
      // Disconnect on GATT Client timeout event.
      NRF_LOG_DEBUG("GATT Client Timeout.");
      APP_ERROR_CHECK(sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
      break;

    case BLE_GATTS_EVT_TIMEOUT:
      // Disconnect on GATT Server timeout event.
      NRF_LOG_DEBUG("GATT Server Timeout.");
      APP_ERROR_CHECK(sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
      break;

    default:
      break;
  }
}

// This function sets up all the necessary GAP (Generic Access Profile) parameters of the
// device including the device name, appearance, and the preferred connection parameters.
static void InitGapParams() {
  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  APP_ERROR_CHECK(sd_ble_gap_device_name_set(&sec_mode,
                                             (const uint8_t*)DEVICE_NAME,
                                             strlen(DEVICE_NAME)));
  ble_gap_conn_params_t gap_conn_params;
  memset(&gap_conn_params, 0, sizeof(gap_conn_params));
  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

  APP_ERROR_CHECK(sd_ble_gap_ppcp_set(&gap_conn_params));
}

static void InitGatt(void) {
  APP_ERROR_CHECK(nrf_ble_gatt_init(&m_gatt, NULL));
}

// Initializes the SoftDevice and the BLE event interrupt.
static void InitBleStack() {
  APP_ERROR_CHECK(nrf_sdh_enable_request());

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  APP_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start));

  // Enable BLE stack.
  APP_ERROR_CHECK(nrf_sdh_ble_enable(&ram_start));

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, BleEventHandler, NULL);
}

// Function for initializing the Advertising functionality.
// Encodes the required advertising data and passes it to the stack.
// Also builds a structure to be passed to the stack when starting advertising.
 static void InitAdvertising() {
  ble_uuid_t adv_uuids[] = {{LBS_UUID_SERVICE, m_lbs.uuid_type}};

  // Build and set advertising data.
  ble_advdata_t advdata;
  memset(&advdata, 0, sizeof(advdata));

  advdata.name_type = BLE_ADVDATA_FULL_NAME;
  advdata.include_appearance = true;
  advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

  ble_advdata_t srdata;
  memset(&srdata, 0, sizeof(srdata));
  srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
  srdata.uuids_complete.p_uuids = adv_uuids;

  APP_ERROR_CHECK(ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len));
  APP_ERROR_CHECK(ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len));

  // Set advertising parameters.
  ble_gap_adv_params_t adv_params;
  memset(&adv_params, 0, sizeof(adv_params));

  adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
  adv_params.duration = APP_ADV_DURATION;
  adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
  adv_params.p_peer_addr = NULL;
  adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
  adv_params.interval = APP_ADV_INTERVAL;

  APP_ERROR_CHECK(sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params));
}

static void LedWriteHandler(uint16_t conn_handle, ble_lbs_t* p_lbs, uint8_t led_state) {
  if (led_state) {
    gBleCallback(true);
    NRF_LOG_INFO("Received LED ON!");
  } else {
    gBleCallback(false);
    NRF_LOG_INFO("Received LED OFF!");
  }
}

static void GenericErrorHandler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

static void InitServices() {
  // Initialize Queued Write Module.
  nrf_ble_qwr_init_t qwr_init = {0};
  qwr_init.error_handler = GenericErrorHandler;
  APP_ERROR_CHECK(nrf_ble_qwr_init(&m_qwr, &qwr_init));

  // Initialize LBS.
  ble_lbs_init_t init = {0};
  init.led_write_handler = LedWriteHandler;
  APP_ERROR_CHECK(ble_lbs_init(&m_lbs, &init));
}

static void InitConnectionParams() {
  ble_conn_params_init_t cp_init;
  memset(&cp_init, 0, sizeof(cp_init));
  cp_init.p_conn_params = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail = true;
  cp_init.error_handler = GenericErrorHandler;
  APP_ERROR_CHECK(ble_conn_params_init(&cp_init));
}

void InitBle(BleCallback callback) {
  gBleCallback = callback;
  InitBleStack();
  InitGapParams();
  InitGatt();
  InitServices();
  InitAdvertising();
  InitConnectionParams();
  StartAdvertising();
  NRF_LOG_INFO("BLE inited, advertising started.");
}
