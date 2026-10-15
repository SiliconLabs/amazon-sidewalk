/***************************************************************************//**
 * @file
 * @brief app_bluetooth.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app_bluetooth.h"
#include "app_init.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led_instances.h"
#include "app_button_press.h"
#include "sl_sidewalk_led_manager.h"
#include "app_process.h"
#include "sl_sidewalk_log_app.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define APP_BLUETOOTH_INVALID_ADV_SET_HANDLE  (0xFF)

#define APP_BLUETOOTH_INDICATION_QUEUE_SIZE 10

typedef struct ble_indication_queue_item{
  uint16_t attribute;
  uint8_t value;
} ble_indication_queue_item_t;

typedef struct ble_indication_queue{
  QueueHandle_t queue;
  bool indication_wait_for_ack;
} ble_indication_queue_t;
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = APP_BLUETOOTH_INVALID_ADV_SET_HANDLE;
// The id of the connection to the mobile app
static uint8_t connection_id = SL_BT_INVALID_CONNECTION_HANDLE;

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
// BLE inited
static bool regular_ble_inited = false;
#endif

static ble_indication_queue_t ble_indication_queue;
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
// Sends notification of the Report Button characteristic.
static sl_status_t send_report_indication(uint16_t attribute, uint8_t value);
// Initializes the indication queue.
static void indication_queue_init(void);
// Enqueues an indication item into the queue.
static bool enqueue_indication(uint16_t attribute, uint8_t value);
// Dequeues an indication item from the queue.
static bool dequeue_indication(uint16_t *attribute, uint8_t *value);
// Clears the indication queue.
static void indication_queue_clear(void);
// Processes the indication queue.
static void indication_queue_process(void);
// Sets the indication ongoing state.
static void set_indication_ongoing(bool ongoing);
// Checks if the indication is ongoing.
static bool is_indication_ongoing(void);
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * BLE application init.
 *****************************************************************************/
void app_bluetooth_init(void)
{
  // Make sure there will be no button events before the boot event.
  app_button_press_disable();

  indication_queue_init();

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
/**************************************************************************//**
 * Get BLE init state.
 *****************************************************************************/
bool app_bluetooth_get_regular_ble_inited(void)
{
  return regular_ble_inited;
}
#endif

/**************************************************************************//**
 * BLE Application Process Action.
 *****************************************************************************/
void app_bluetooth_update_led_status(uint8_t value, enum toggle_led_source source)
{
  uint16_t attribute;

  // Mobile app is not connected to send indications
  if (connection_id == SL_BT_INVALID_CONNECTION_HANDLE) {
    return;
  }

  attribute = gattdb_light_state_sidewalk;
  if (!enqueue_indication(attribute, value)) {
    SL_SID_LOG_APP_ERROR("Failed to enqueue indication for attribute: %d, value: 0x%02x", attribute, value);
    return;
  }

  attribute = gattdb_trigger_source_sidewalk;
  if (!enqueue_indication(attribute, (uint8_t)source)) {
    SL_SID_LOG_APP_ERROR("Failed to enqueue indication for attribute: %d, value: 0x%02x", attribute, (uint8_t)source);
    return;
  }

  indication_queue_process();
}

/**************************************************************************//**
 * BLE init and start advertisement
 *****************************************************************************/
void app_bluetooth_init_and_start_advertisement(void)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;

  // Extract unique ID from BT Address.
  sc = sl_bt_system_get_identity_address(&address, &address_type);
  app_assert_status(sc);

  SL_SID_LOG_APP_INFO("BLE %s address: %02X:%02X:%02X:%02X:%02X:%02X",
                      address_type ? "static random" : "public device",
                      address.addr[5],
                      address.addr[4],
                      address.addr[3],
                      address.addr[2],
                      address.addr[1],
                      address.addr[0]);

  // Create an advertising set.
  sc = sl_bt_advertiser_create_set(&advertising_set_handle);
  app_assert_status(sc);

  // Generate data for advertising
  sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                             sl_bt_advertiser_general_discoverable);
  app_assert_status(sc);

  // Set advertising interval to 1000ms.
  sc = sl_bt_advertiser_set_timing(advertising_set_handle,
                                   1600,  // min. adv. interval (milliseconds * 1.6)
                                   1600,  // max. adv. interval (milliseconds * 1.6)
                                   0,     // adv. duration
                                   0);    // max. num. adv. events
  app_assert_status(sc);

  // Start advertising and enable connections.
  sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                     sl_bt_legacy_advertiser_connectable);
  app_assert_status(sc);

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
  regular_ble_inited = true;

  #if (SL_SIMPLE_LED_COUNT >= 2)
  sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
  #endif
#endif

  SL_SID_LOG_APP_INFO("BLE advertising started");
}

/**************************************************************************//**
 * BLE start advertisement
 *****************************************************************************/
void app_bluetooth_start_advertisement(void)
{
  sl_status_t sc;

  // Generate data for advertising
  sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                             sl_bt_advertiser_general_discoverable);
  app_assert_status(sc);

  // Restart advertising after client has disconnected.
  sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                     sl_bt_legacy_advertiser_connectable);
  app_assert_status(sc);

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
  #if (SL_SIMPLE_LED_COUNT >= 2)
  sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
  #endif
#endif
}

/**************************************************************************//**
 * BLE toggle LED write response
 *****************************************************************************/
void app_bluetooth_toggle_led_write_response(bool success)
{
  sl_status_t sc;
  sc = sl_bt_gatt_server_send_user_write_response(connection_id,
                                                  gattdb_light_state_sidewalk,
                                                  success ? 0 : 1);
  app_log_status_error(sc);
}

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
/**************************************************************************//**
 * BLE stop advertisement
 *****************************************************************************/
void app_bluetooth_stop_advertisement(void)
{
  // Stop advertising
  sl_status_t sc = sl_bt_advertiser_stop(advertising_set_handle);
  app_assert_status(sc);

  #if (SL_SIMPLE_LED_COUNT >= 2)
  sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(1));
  #endif
}
#endif

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint16_t sent_len;

  switch (SL_BT_MSG_ID(evt->header)) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      SL_SID_LOG_APP_INFO("BLE boot: v%d.%d.%d-b%d",
                          evt->data.evt_system_boot.major,
                          evt->data.evt_system_boot.minor,
                          evt->data.evt_system_boot.patch,
                          evt->data.evt_system_boot.build);

      app_bluetooth_init_and_start_advertisement();
      break;
#endif
    // -------------------------------

    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      // To check if the connection was opened from the mobile app
      if (advertising_set_handle == evt->data.evt_connection_opened.advertiser) {
        SL_SID_LOG_APP_INFO("BLE connection opened");
        connection_id = evt->data.evt_connection_opened.connection;
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
        sc = sl_bt_connection_set_parameters(evt->data.evt_connection_opened.connection,
                                             80,     // min. con. interval (milliseconds * 1.25)
                                             80,     // max. con. interval (milliseconds * 1.25)
                                             0,      // latency
                                             100,    // timeout (milliseconds * 10)
                                             0x0,    // min. connection event length (milliseconds * 0.625)
                                             0xffff); // max. connection event length (milliseconds * 0.625)
        app_assert_status(sc);

        #ifdef SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT
        // Set remote connection power reporting - needed for Power Control
        sc = sl_bt_connection_set_remote_power_reporting(evt->data.evt_connection_opened.connection,
                                                         sl_bt_connection_power_reporting_enable);
        app_assert_status(sc);
        #endif // SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT
#endif
      }
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      if (connection_id == evt->data.evt_connection_closed.connection) {
        SL_SID_LOG_APP_INFO("BLE connection closed");
        connection_id = SL_BT_INVALID_CONNECTION_HANDLE;
        indication_queue_clear();
        if (advertising_set_handle != APP_BLUETOOTH_INVALID_ADV_SET_HANDLE) {
          app_bluetooth_start_advertisement();
          SL_SID_LOG_APP_INFO("BLE advertising started");
        }
      }
      break;

    case sl_bt_evt_gatt_server_user_read_request_id:
      if (gattdb_light_state_sidewalk == evt->data.evt_gatt_server_user_read_request.characteristic) {
        app_led_status_t led_status = app_get_led_status();
        SL_SID_LOG_APP_INFO("BLE user read request for light state, value: 0x%02x", led_status.value);
        sc = sl_bt_gatt_server_send_user_read_response(evt->data.evt_gatt_server_user_read_request.connection,
                                                       gattdb_light_state_sidewalk,
                                                       0,
                                                       sizeof(uint8_t),
                                                       &led_status.value,
                                                       &sent_len);
        app_log_status_error(sc);
      } else if (gattdb_trigger_source_sidewalk == evt->data.evt_gatt_server_user_read_request.characteristic) {
        app_led_status_t led_status = app_get_led_status();
        SL_SID_LOG_APP_INFO("BLE user read request for trigger source, value: 0x%02x", led_status.source);
        sc = sl_bt_gatt_server_send_user_read_response(evt->data.evt_gatt_server_user_read_request.connection,
                                                       gattdb_trigger_source_sidewalk,
                                                       0,
                                                       sizeof(uint8_t),
                                                       &led_status.source,
                                                       &sent_len);
        app_log_status_error(sc);
      } else if (gattdb_source_address_sidewalk == evt->data.evt_gatt_server_user_read_request.characteristic) {
        // Always return unknown source address
        uint8_t source_address[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        SL_SID_LOG_APP_INFO("BLE user read request for source address, value: 0x%02x", source_address);
        sc = sl_bt_gatt_server_send_user_read_response(evt->data.evt_gatt_server_user_read_request.connection,
                                                       gattdb_source_address_sidewalk,
                                                       0,
                                                       sizeof(source_address),
                                                       source_address,
                                                       &sent_len);
        app_log_status_error(sc);
      }
      break;
    // -------------------------------
    // This event indicates either that a local Client Characteristic Configuration descriptor
    // has been changed by the remote GATT client, or that a confirmation from the remote GATT
    // client was received upon a successful reception of the indication.
    case sl_bt_evt_gatt_server_characteristic_status_id:
      if ((sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags)
          && ((gattdb_light_state_sidewalk == evt->data.evt_gatt_server_characteristic_status.characteristic)
              || (gattdb_trigger_source_sidewalk == evt->data.evt_gatt_server_characteristic_status.characteristic))) {
        set_indication_ongoing(false);
        indication_queue_process();
      }
      break;

    // -------------------------------
    // This event indicates that a remote GATT client is attempting to write
    // a value of a user type attribute in to the local GATT database.
    case sl_bt_evt_gatt_server_user_write_request_id:
      // light state write
      if (gattdb_light_state_sidewalk == evt->data.evt_gatt_server_user_write_request.characteristic) {
        uint8_t data_recv = evt->data.evt_gatt_server_user_write_request.value.data[0];
#if defined(SL_SID_APP_MSG_PRESENT)
        sl_sid_app_msg_dmp_soc_light_toggle_led_ctx_t ctx = {
          .param_send.state = data_recv,
          .hdl.operation = SL_SID_APP_MSG_OP_NTFY
        };
        app_trigger_toggle_led(&ctx, TOGGLE_LED_SOURCE_BLE);
#else
        (void)data_recv;
#endif
      }
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Sends indication of the Report Button characteristic.
 *
 * Reads the current button state from the local GATT database and sends it as a
 * indication.
 ******************************************************************************/
static sl_status_t send_report_indication(uint16_t attribute, uint8_t value)
{
  sl_status_t sc;

  // Send characteristic indication.
  sc = sl_bt_gatt_server_send_indication(connection_id,
                                         attribute,
                                         sizeof(value),
                                         &value);
  if (sc == SL_STATUS_OK) {
    SL_SID_LOG_APP_INFO("BLE report indication sent, attribute: %d, data: 0x%02x", attribute, value);
  }
  return sc;
}

static void indication_queue_init(void)
{
  ble_indication_queue.indication_wait_for_ack = false;
  ble_indication_queue.queue = xQueueCreate(APP_BLUETOOTH_INDICATION_QUEUE_SIZE, sizeof(ble_indication_queue_item_t));
  app_assert(ble_indication_queue.queue != NULL, "Failed to create indication queue");
}

static bool is_indication_ongoing(void)
{
  return ble_indication_queue.indication_wait_for_ack;
}

static void set_indication_ongoing(bool ongoing)
{
  ble_indication_queue.indication_wait_for_ack = ongoing;
}

static bool enqueue_indication(uint16_t attribute, uint8_t value)
{
  ble_indication_queue_item_t item = { attribute, value };

  if (xQueueSend(ble_indication_queue.queue, &item, 0) == pdPASS) {
    return true;
  }
  return false;
}

static bool dequeue_indication(uint16_t *attribute, uint8_t *value)
{
  ble_indication_queue_item_t item;

  if (xQueueReceive(ble_indication_queue.queue, &item, 0) == pdPASS) {
    *attribute = item.attribute;
    *value = item.value;
    return true;
  }
  return false;
}

static void indication_queue_clear(void)
{
  ble_indication_queue.indication_wait_for_ack = false;
  xQueueReset(ble_indication_queue.queue);
}

static void indication_queue_process(void)
{
  uint16_t attribute;
  uint8_t value;
  if (is_indication_ongoing()) {
    return;
  }
  if (dequeue_indication(&attribute, &value)) {
    set_indication_ongoing(true);
    send_report_indication(attribute, value);
  }
}
