/***************************************************************************//**
 * @file
 * @brief app_bluetooth.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "em_common.h"
#include "app_assert.h"
#include "app_log.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app_bluetooth.h"
#include "app_init.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led_instances.h"
#include "app_button_press.h"
#include "sl_sidewalk_led_manager.h"
#include "app_process.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xFF;

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
// Updates the Report Button characteristic.
static sl_status_t update_report_characteristic(uint16_t attribute, uint8_t value);
// Sends notification of the Report Button characteristic.
static sl_status_t send_report_notification(uint16_t attribute);

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

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * BLE Application Process Action.
 *****************************************************************************/
void app_bluetooth_update_led_status(uint8_t value)
{
  sl_status_t sc;
  uint16_t attribute = gattdb_report_button;

  sc = update_report_characteristic(attribute, value);
  app_log_status_error(sc);

  if (sc == SL_STATUS_OK) {
    sc = send_report_notification(attribute);
    app_log_status_error(sc);
  }
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      app_log_info("app: BLE booted: v%d.%d.%d-b%d\n",
                   evt->data.evt_system_boot.major,
                   evt->data.evt_system_boot.minor,
                   evt->data.evt_system_boot.patch,
                   evt->data.evt_system_boot.build);

      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      app_log_info("app: BLE %s addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
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
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        1600, // min. adv. interval (milliseconds * 1.6)
        1600, // max. adv. interval (milliseconds * 1.6)
        0,    // adv. duration
        0);   // max. num. adv. events
      app_assert_status(sc);

      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);

      app_log_info("app: started adv");

      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log_info("app: conn opened");

      sc = sl_bt_connection_set_parameters(
        evt->data.evt_connection_opened.connection,
        80,      // min. con. interval (milliseconds * 1.25)
        80,      // max. con. interval (milliseconds * 1.25)
        0,       // latency
        100,     // timeout (milliseconds * 10)
        0x0,     // min. connection event length (milliseconds * 0.625)
        0xffff); // max. connection event length (milliseconds * 0.625)
      app_assert_status(sc);

#ifdef SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT
      // Set remote connection power reporting - needed for Power Control
      sc = sl_bt_connection_set_remote_power_reporting(
        evt->data.evt_connection_opened.connection,
        sl_bt_connection_power_reporting_enable);
      app_assert_status(sc);
#endif // SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT

      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log_info("app: conn closed");

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);

      app_log_info("app: started adv");

      break;

    // -------------------------------
    // This event indicates that the value of an attribute in the local GATT
    // database was changed by a remote GATT client.
    case sl_bt_evt_gatt_server_attribute_value_id:
      // The value of the gattdb_led_control characteristic was changed.
      if (gattdb_led_control == evt->data.evt_gatt_server_attribute_value.attribute) {
        uint8_t data_recv;
        size_t data_recv_len;

        // Read characteristic value.
        sc = sl_bt_gatt_server_read_attribute_value(gattdb_led_control,
                                                    0,
                                                    sizeof(data_recv),
                                                    &data_recv_len,
                                                    &data_recv);
        (void)data_recv_len;
        app_log_status_error(sc);

        if (sc != SL_STATUS_OK) {
          break;
        }
#if defined(SL_SID_APP_MSG_PRESENT)
        // Toggle LED.
        sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t ctx = {
          .param_send.led = data_recv,
          .hdl.operation = SL_SID_APP_MSG_OP_NTFY
        };
        app_trigger_toggle_led(&ctx);
#else
        (void)data_recv;
#endif
      }
      break;

    // -------------------------------
    // This event occurs when the remote device enabled or disabled the
    // notification.
    case sl_bt_evt_gatt_server_characteristic_status_id:
      if (gattdb_report_button == evt->data.evt_gatt_server_characteristic_status.characteristic) {
        // A local Client Characteristic Configuration descriptor was changed in
        // the gattdb_report_button characteristic.
        if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
            & sl_bt_gatt_notification) {
          // The client just enabled the notification. Send notification of the
          // current button state stored in the local GATT table.
          app_log_info("app: notif enabled");

          sc = send_report_notification(gattdb_report_button);
          app_log_status_error(sc);
        } else {
          app_log_info("app: notif disabled");
        }
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

// Sends notification of the Report Button characteristic.

/***************************************************************************//**
 * Updates the Report Button characteristic.
 *
 * Checks the current button state and then writes it into the local GATT table.
 ******************************************************************************/
static sl_status_t update_report_characteristic(uint16_t attribute, uint8_t data_send)
{
  sl_status_t sc;

  // Write attribute in the local GATT database.
  sc = sl_bt_gatt_server_write_attribute_value(attribute,
                                               0,
                                               sizeof(data_send),
                                               &data_send);
  if (sc == SL_STATUS_OK) {
    app_log_info("app: update btn characteristic (%d) : attr written: 0x%02x", attribute, (int)data_send);
  }

  return sc;
}

/***************************************************************************//**
 * Sends notification of the Report Button characteristic.
 *
 * Reads the current button state from the local GATT database and sends it as a
 * notification.
 ******************************************************************************/
static sl_status_t send_report_notification(uint16_t attribute)
{
  sl_status_t sc;
  uint8_t data_send;
  size_t data_len;

  // Read report button characteristic stored in local GATT database.
  sc = sl_bt_gatt_server_read_attribute_value(attribute,
                                              0,
                                              sizeof(data_send),
                                              &data_len,
                                              &data_send);
  if (sc != SL_STATUS_OK) {
    return sc;
  }

  // Send characteristic notification.
  sc = sl_bt_gatt_server_notify_all(attribute,
                                    sizeof(data_send),
                                    &data_send);
  if (sc == SL_STATUS_OK) {
    app_log_append("app: send report notif (%d) : notif sent: 0x%02x", attribute, (int)data_send);
  }
  return sc;
}
