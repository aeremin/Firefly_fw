#include "bluetooth.h"

#include "app_timer.h"
#include "app_error.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_log.h"

// NRF_SDH_BLE_OBSERVER uses _Static_assert (which is C11 feature).
// But it is not available when building as C++ code. So we do this hack-define to fix compilability.
#define _Static_assert(EXPR, MSG) static_assert(EXPR, MSG);

BluetoothLowEnergy::BleCallback BluetoothLowEnergy::ble_callback = nullptr;
ble_lbs_t BluetoothLowEnergy::ble_led_button_service_;
nrf_ble_gatt_t BluetoothLowEnergy::gatt_;

void GlobalBleEventHandler(ble_evt_t const* p_ble_evt, void* p_context) {
  reinterpret_cast<BluetoothLowEnergy*>(p_context)->BleEventHandler(p_ble_evt);
}

BluetoothLowEnergy::BluetoothLowEnergy()
: conn_handle_(BLE_CONN_HANDLE_INVALID),
  adv_handle_(BLE_GAP_ADV_SET_HANDLE_NOT_SET),
  adv_data_({
    /* adv_data = */ {
      /* p_data = */ enc_advdata_,
      /* len = */ BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    /* scan_rsp_data = */ {
      /* p_data = */ enc_scan_response_data_,
      /* len = */ BLE_GAP_ADV_SET_DATA_SIZE_MAX
    }
  }) {
  NRF_SDH_BLE_OBSERVER(m_lbs_obs, BLE_LBS_BLE_OBSERVER_PRIO, ble_lbs_on_ble_evt, &BluetoothLowEnergy::ble_led_button_service_);
  NRF_SDH_BLE_OBSERVER(m_gatt_obs, NRF_BLE_GATT_BLE_OBSERVER_PRIO, nrf_ble_gatt_on_ble_evt, &BluetoothLowEnergy::gatt_);
  // Register a handler for BLE events.
  // All system observers have priority 1 or 2 (as defined in sdk_config.h).
  const uint8_t observer_priority = 3;
  NRF_SDH_BLE_OBSERVER(m_ble_observer, observer_priority, GlobalBleEventHandler, this);
}

void BluetoothLowEnergy::StartAdvertising() {
  APP_ERROR_CHECK(sd_ble_gap_adv_start(adv_handle_, connection_configuraion_tag_));
}

void BluetoothLowEnergy::BleEventHandler(ble_evt_t const* p_ble_evt) {
  switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
      NRF_LOG_INFO("Connected");
      conn_handle_ = p_ble_evt->evt.gap_evt.conn_handle;
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      NRF_LOG_INFO("Disconnected");
      conn_handle_ = BLE_CONN_HANDLE_INVALID;
      StartAdvertising();
      break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
      // Pairing not supported
      APP_ERROR_CHECK(sd_ble_gap_sec_params_reply(conn_handle_,
                                                  BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                  nullptr,
                                                  nullptr));
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
      APP_ERROR_CHECK(sd_ble_gatts_sys_attr_set(conn_handle_, nullptr, 0, 0));
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
void BluetoothLowEnergy::InitGapParams() {
  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  const char device_name[] = "Nordic_Blinky";
  APP_ERROR_CHECK(sd_ble_gap_device_name_set(&sec_mode,
                                             (const uint8_t*)device_name,
                                             strlen(device_name)));
  ble_gap_conn_params_t gap_conn_params;
  memset(&gap_conn_params, 0, sizeof(gap_conn_params));
  // Minimum acceptable connection interval (100ms).
  gap_conn_params.min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS);
  // Maximum acceptable connection interval (200ms).
  gap_conn_params.max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS);
  gap_conn_params.slave_latency = 0;
  // Connection supervisory time-out (4 seconds).
  gap_conn_params.conn_sup_timeout = MSEC_TO_UNITS(4000, UNIT_10_MS);

  APP_ERROR_CHECK(sd_ble_gap_ppcp_set(&gap_conn_params));
}

void BluetoothLowEnergy::InitGatt(void) {
  APP_ERROR_CHECK(nrf_ble_gatt_init(&gatt_, nullptr));
}

// Initializes the SoftDevice and the BLE event interrupt.
void BluetoothLowEnergy::InitBleStack() {
  APP_ERROR_CHECK(nrf_sdh_enable_request());

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  APP_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(connection_configuraion_tag_, &ram_start));

  // Enable BLE stack.
  APP_ERROR_CHECK(nrf_sdh_ble_enable(&ram_start));
}

// Function for initializing the Advertising functionality.
// Encodes the required advertising data and passes it to the stack.
// Also builds a structure to be passed to the stack when starting advertising.
void BluetoothLowEnergy::InitAdvertising() {
  ble_uuid_t adv_uuids[] = {{LBS_UUID_SERVICE, ble_led_button_service_.uuid_type}};

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

  APP_ERROR_CHECK(ble_advdata_encode(&advdata, adv_data_.adv_data.p_data, &adv_data_.adv_data.len));
  APP_ERROR_CHECK(ble_advdata_encode(&srdata, adv_data_.scan_rsp_data.p_data, &adv_data_.scan_rsp_data.len));

  // Set advertising parameters.
  ble_gap_adv_params_t adv_params;
  memset(&adv_params, 0, sizeof(adv_params));

  adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
  adv_params.duration = BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED;
  adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
  adv_params.p_peer_addr = nullptr;
  adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
  // The advertising interval (40 ms).
  adv_params.interval = MSEC_TO_UNITS(40, UNIT_0_625_MS);

  APP_ERROR_CHECK(sd_ble_gap_adv_set_configure(&adv_handle_, &adv_data_, &adv_params));
};

static void LedWriteHandler(uint16_t conn_handle, ble_lbs_t* p_lbs, uint8_t led_state) {
  if (led_state) {
    BluetoothLowEnergy::ble_callback(true);
    NRF_LOG_INFO("Received LED ON!");
  } else {
    BluetoothLowEnergy::ble_callback(false);
    NRF_LOG_INFO("Received LED OFF!");
  }
}

static void GenericErrorHandler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

void BluetoothLowEnergy::InitServices() {
  // Initialize LED Button Service (LBS).
  ble_lbs_init_t init = {0};
  init.led_write_handler = LedWriteHandler;
  APP_ERROR_CHECK(ble_lbs_init(&ble_led_button_service_, &init));
}

void BluetoothLowEnergy::InitConnectionParams() {
  ble_conn_params_init_t cp_init;
  memset(&cp_init, 0, sizeof(cp_init));
  cp_init.p_conn_params = nullptr;
  // Time from initiating event (connect or start of notification) to first time 
  // sd_ble_gap_conn_param_update is called (20 seconds).
  cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(20000);
  // Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds).
  cp_init.next_conn_params_update_delay = APP_TIMER_TICKS(5000);
  cp_init.max_conn_params_update_count = 3;
  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail = true;
  cp_init.error_handler = GenericErrorHandler;
  APP_ERROR_CHECK(ble_conn_params_init(&cp_init));
}

void BluetoothLowEnergy::Init(BleCallback callback) {
  ble_callback = callback;
  InitBleStack();
  InitGapParams();
  InitGatt();
  InitServices();
  InitAdvertising();
  InitConnectionParams();
  StartAdvertising();
  NRF_LOG_INFO("BLE inited, advertising started.");
}
