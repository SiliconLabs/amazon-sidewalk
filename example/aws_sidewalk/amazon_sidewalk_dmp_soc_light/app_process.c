/***************************************************************************//**
 * @file
 * @brief app_process.c
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
#include <stdio.h>
#include <string.h>

#include "app_process.h"
#include "app_init.h"
#include "app_assert.h"
#include "sid_api.h"
#include "sl_bt_api.h"
#include "sl_sidewalk_common_config.h"
#include "sl_sidewalk_utils.h"
#include "app_button_press.h"
#include "sl_sidewalk_nvm3_handler.h"

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
#include "app_subghz_config.h"
#endif

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "sl_simple_button_instances.h"
#endif

#if defined(SL_CATALOG_SIMPLE_LED_PRESENT)
#include "sl_simple_led_instances.h"
#endif

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
#include "app_ble_config.h"
#include "sl_bt_api.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Maximum number Queue elements
#define MSG_QUEUE_LEN           (10U)
// Key value for NVM data
enum app_nvm3_keys {
  DMP_NVM3_KEY_BLE_STATE = SLI_SID_NVM3_KEY_MIN_APP
};

// Format string for send counter log
#define SID_SEND_COUNTER_FORMAT_STR \
 "link type: %x,\
 msg id: %u,\
 msg size: %u,\
 msg type: %u,\
 ack requested: %d,\
 ttl: %d,\
 max retry: %d,\
 additional attr: %d"

// Format string for received message log
#define SID_RECEIVED_MSG_FORMAT_STR \
 "link type: %x,\
 msg id: %u,\
 msg size: %u,\
 msg type: %d,\
 ack requested: %d,\
 is ack: %d,\
 is duplicate: %d,\
 rssi: %d,\
 snr: %d"
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Add an event to the event queue.
 *
 * @param[in] queue The queue handle to which the event will be added
 * @param[in] event The event to be added
 ******************************************************************************/
static void queue_event(QueueHandle_t queue, enum event_type event);

#if defined(SL_SID_APP_MSG_PRESENT)
/*******************************************************************************
 * Callback function triggered when device reset timer fires
 *
 * @param[in] tmr_hdl Timer handle
 ******************************************************************************/
static void dev_reset_timer_cb(TimerHandle_t tmr_hdl);

/*******************************************************************************
 * Function to execute device reset command
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void exec_device_reset(app_context_t *app_ctx);

/*******************************************************************************
 * Function to execute sending button press response
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void exec_send_button_press_resp(app_context_t *app_ctx);

/*******************************************************************************
 * Function to execute BLE start/stop command
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void exec_ble_start_stop(app_context_t *app_ctx);

/*******************************************************************************
 * Function to execute counter update command
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void exec_counter_update(app_context_t *app_ctx);

/*******************************************************************************
 * Function to execute time command
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void exec_time(app_context_t *app_ctx);

/*******************************************************************************
 * Function to execute MTU command
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void exec_mtu(app_context_t *app_ctx);

/*******************************************************************************
 * Function to convert application message into sidewalk message and to send it
 * over the sidewalk network
 *
 * @param[in] app_ctx Application context
 * @param[in] send_app_msg Message to be sent
 ******************************************************************************/
static void send_message(app_context_t *app_ctx, sl_sid_app_msg_t *send_app_msg);
#endif

/*******************************************************************************
 * Callback for sidewalk message event
 *
 * @param[in] in_isr If the event shall be handled from ISR context
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_event(bool in_isr, void *context);

/*******************************************************************************
 * Callback for sidewalk message reception
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] msg The received message
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context);

/*******************************************************************************
 * Callback for sidewalk message transmission
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context);

/*******************************************************************************
 * Callback for sidewalk message transmission error
 *
 * @param[in] error The error type
 * @param[in] msg_desc Message descriptor
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context);

/*******************************************************************************
 * Callback for sidewalk status change
 *
 * @param[in] status New status
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context);

/*******************************************************************************
 * Callback for sidewalk factory reset ready event
 *
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_factory_reset(void *context);

/*******************************************************************************
 * Function to convert link_type configuration to sidewalk stack link_mask
 *
 * @param[in] link_type link_type configuration to convert
 *
 * @return link_mask Corresponding link_mask enumeration
 ******************************************************************************/
static uint32_t link_type_to_link_mask(uint8_t link_type);

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
/*******************************************************************************
 * Function to init and start regular ble link
 *
 * @param[in] app_ctx Application context
 *
 * @return 0 on success, -1 otherwise
 ******************************************************************************/
static int32_t init_and_start_regular_ble(app_context_t *app_ctx);

/*******************************************************************************
 * Function to set FSK power profile 2
 *
 * @param[in] app_ctx Application context
 ******************************************************************************/
static void set_fsk_profile_2(app_context_t *app_ctx);
#endif

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
/*******************************************************************************
 * Function to init and start regular ble link advertisement
 *
 * @param[in] app_ctx Application context
 *
 * @return None
 ******************************************************************************/
static void init_and_start_regular_ble_advertisement(app_context_t *app_ctx);

/*******************************************************************************
 * Function to trigger the connection request towards GW
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void toggle_connection_request(app_context_t *context);
#endif

/*******************************************************************************
 * Function to init and start sidewalk links
 *
 * @param[out] app_ctx Application context
 * @param[out] config Sidewalk configuration parameters
 * @param[in] link_mask Sidewalk stack link mask
 *
 * @return 0 on success, -1 otherwise
 ******************************************************************************/
static int32_t init_and_start_link(app_context_t *app_ctx,
                                   struct sid_config *config,
                                   uint32_t link_mask);

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
/*******************************************************************************
 * Function to trigger the connection request towards GW
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void toggle_connection_request(app_context_t *context);
#endif

/*******************************************************************************
 * Function to update the LED status
 *
 * @param[in] app_ctx Application context
 * @param[in] led_status LED status to update
 ******************************************************************************/
static void update_led_status(app_context_t *app_ctx, app_led_status_t led_status);

/*******************************************************************************
 * Function to get the LED status
 *
 * @param[in] app_ctx Application context
 *
 * @return LED status
 ******************************************************************************/
static app_led_status_t get_led_status(app_context_t *app_ctx);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// Sidewalk application context
static app_context_t g_app_ctx = {
  .state = STATE_SIDEWALK_NOT_READY
};

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
// Button send update request
static bool btn_send_update_req = false;
#endif

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
// FSK profile 2 switch request pending
static bool fsk_profile_2_requested = false;
#endif
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void main_thread(void *context)
{
  // Sidewalk events
  enum event_type event = EVENT_TYPE_INVALID;

  (void)context;

  // Register the callback functions and the context
  static struct sid_event_callbacks event_callbacks =
  {
    .context           = &g_app_ctx,
    .on_event          = on_sidewalk_event,          // Called from ISR context
    .on_msg_received   = on_sidewalk_msg_received,   // Called from sid_process()
    .on_msg_sent       = on_sidewalk_msg_sent,       // Called from sid_process()
    .on_send_error     = on_sidewalk_send_error,     // Called from sid_process()
    .on_status_changed = on_sidewalk_status_changed, // Called from sid_process()
    .on_factory_reset  = on_sidewalk_factory_reset,  // Called from sid_process()
  };

  // Set configuration parameters
  static struct sid_config config =
  {
    .link_mask = 0,
    .dev_ch = {
      .type = SID_END_DEVICE_TYPE_STATIC,
      .power_type = SID_END_DEVICE_POWERED_BY_LINE_POWER_ONLY,
      .qualification_id = 0x0002,
    },
    .callbacks = &event_callbacks,
    .link_config = NULL,
    .sub_ghz_link_config = NULL,
  };

  // Queue creation for the sidewalk events
  g_app_ctx.event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(g_app_ctx.event_queue != NULL, "queue creation failed");

  g_app_ctx.led_status_semaphore = xSemaphoreCreateBinary();
  app_assert(g_app_ctx.led_status_semaphore != NULL, "semaphore creation failed");
  xSemaphoreGive(g_app_ctx.led_status_semaphore);

#if defined(SL_SID_APP_MSG_PRESENT)
  // Timer creation for the device reset
  g_app_ctx.device_reset_timer = xTimerCreate("tmr",
                                              1,
                                              pdFALSE,
                                              (void *)0,
                                              dev_reset_timer_cb);
  if (g_app_ctx.device_reset_timer == NULL) {
    SL_SID_LOG_APP_ERROR("device reset timer creation failed");
    goto error;
  }
#endif

#if defined(SL_BLE_SUPPORTED)
  SL_SID_LOG_APP_INFO("BLE link supported");
#endif
#if defined(SL_FSK_SUPPORTED)
  SL_SID_LOG_APP_INFO("FSK link supported");
#endif
#if defined(SL_CSS_SUPPORTED)
  SL_SID_LOG_APP_INFO("CSS link supported");
#endif

#if defined (SV_ENABLED)
  SL_SID_LOG_APP_INFO("Secure Vault is enabled");
#else
  SL_SID_LOG_APP_INFO("Secure Vault is disabled");
#endif

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  config.sub_ghz_link_config = app_get_sub_ghz_config();
#endif

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
  config.link_config = app_get_ble_config();
#endif

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  // Start regular BLE if needed
  if (init_and_start_regular_ble(&g_app_ctx) != 0) {
    goto error;
  }
#endif

  // Initialize and start Sidewalk FSK
  if (init_and_start_link(&g_app_ctx,
                          &config,
                          link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK)) != 0) {
    goto error;
  }

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
  // Start regular BLE advertisement if needed
  init_and_start_regular_ble_advertisement(&g_app_ctx);
#endif

  SL_SID_LOG_APP_INFO("main task started");

  while (1) {
    if (xQueueReceive(g_app_ctx.event_queue, &event, portMAX_DELAY) == pdTRUE) {
      // State machine for Sidewalk events
      switch (event) {
        case EVENT_TYPE_SID_PROCESS_NEEDED:
          SL_SID_LOG_APP_DEBUG("sidewalk process event");
          sid_process(g_app_ctx.sidewalk_handle);
          break;

        case EVENT_TYPE_COUNTER_UPDATE:
          SL_SID_LOG_APP_INFO("counter update event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          exec_counter_update(&g_app_ctx);
          #endif
          break;

        case EVENT_TYPE_TIME:
          SL_SID_LOG_APP_INFO("get time event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          exec_time(&g_app_ctx);
          #endif
          break;

        case EVENT_TYPE_MTU:
          SL_SID_LOG_APP_INFO("get MTU event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          exec_mtu(&g_app_ctx);
          #endif
          break;

        case EVENT_TYPE_DEVICE_RESET:
          SL_SID_LOG_APP_INFO("factory reset event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          exec_device_reset(&g_app_ctx);
          #endif
          break;

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
        case EVENT_TYPE_SET_FSK_PROFILE_2:
          SL_SID_LOG_APP_INFO("set FSK profile 2 event");
          set_fsk_profile_2(&g_app_ctx);
          break;
#endif

        case EVENT_TYPE_DEV_REGISTERED:
          SL_SID_LOG_APP_INFO("device registered event");
          if ((SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE != SL_SIDEWALK_COMMON_REGISTRATION_LINK)
              && (init_and_start_link(&g_app_ctx,
                                     &config,
                                     link_type_to_link_mask(SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE)) != 0)) {
            goto error;
          }
          break;

        case EVENT_TYPE_TOGGLE_LED_SOURCE_BOARD:
          SL_SID_LOG_APP_INFO("toggle LED event from board");
          #if defined(SL_SID_APP_MSG_PRESENT)
          sl_sidewalk_led_manager_toggle_led(0, SL_SIDEWALK_LED_MANAGER_SOURCE_BOARD);
          #endif
          break;

        case EVENT_TYPE_TOGGLE_LED_SOURCE_BLE:
          SL_SID_LOG_APP_INFO("toggle LED event from BLE");
          #if defined(SL_SID_APP_MSG_PRESENT)
          sl_sidewalk_led_manager_toggle_led(0, SL_SIDEWALK_LED_MANAGER_SOURCE_BLE);
          #endif
          break;

        case EVENT_TYPE_TOGGLE_LED_SOURCE_CLOUD:
          SL_SID_LOG_APP_INFO("toggle LED event from cloud");
          #if defined(SL_SID_APP_MSG_PRESENT)
          sl_sidewalk_led_manager_toggle_led(0, SL_SIDEWALK_LED_MANAGER_SOURCE_CLOUD);
          #endif
          break;

        case EVENT_TYPE_BLE_START_STOP:
          SL_SID_LOG_APP_INFO("BLE start/stop event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          exec_ble_start_stop(&g_app_ctx);
          #endif
          break;

        case EVENT_TYPE_BTN_PRESS:
          SL_SID_LOG_APP_INFO("button press event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          // if btn press is received as an RTOS event then it means that it's emulation
          // real btn press triggers btn press callback and not an RTOS event
          g_app_ctx.app_msg.button_press_ctx.is_emulation = true;
          // trigger btn press callback as if user pressed the button
          app_button_press_cb(g_app_ctx.app_msg.button_press_ctx.param_send.button,
                              g_app_ctx.app_msg.button_press_ctx.param_send.duration);
          #endif
          break;

        case EVENT_TYPE_BTN_PRESS_SEND_RESP:
          SL_SID_LOG_APP_INFO("send button press response event");
          #if defined(SL_SID_APP_MSG_PRESENT)
          exec_send_button_press_resp(&g_app_ctx);
          #endif
          break;

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
        case EVENT_TYPE_CONNECTION_REQUEST:
          SL_SID_LOG_APP_INFO("BLE connection request event");
          toggle_connection_request(&g_app_ctx);
          break;
#endif
        default:
          SL_SID_LOG_APP_ERROR("unexpected event: %u", event);
          break;
      }
    }
  }

  error:
  // If error happens deinit sidewalk
  if (g_app_ctx.sidewalk_handle != NULL) {
    sid_stop(g_app_ctx.sidewalk_handle, config.link_mask);
    sid_deinit(g_app_ctx.sidewalk_handle);
    g_app_ctx.sidewalk_handle = NULL;
  }
  SL_SID_LOG_APP_ERROR("unrecoverable error occurred");

  sid_platform_deinit();
  vTaskDelete(NULL);
}

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
void app_trigger_connect_and_send(void)
{
  if (g_app_ctx.state != STATE_SIDEWALK_READY) {
    if (!btn_send_update_req) {
      btn_send_update_req = true;
      app_trigger_connection_request();
    } else {
      SL_SID_LOG_APP_WARNING("connection request already in progress");
    }
  } else {
    sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t ctx = {
      .hdl.operation = SL_SID_APP_MSG_OP_NTFY
    };
    app_trigger_update_counter(&ctx);
  }
}

static void toggle_connection_request(app_context_t *context)
{
  if (context->state == STATE_SIDEWALK_READY) {
    SL_SID_LOG_APP_WARNING("BLE connection is already established");
  } else {
    context->connection_request = true;
    sid_error_t ret = sid_ble_bcn_connection_request(context->sidewalk_handle,
                                                     context->connection_request);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("BLE connection request failed: %d", ret);
    }

    SL_SID_LOG_APP_INFO("BLE connection request set");
  }
}

void app_trigger_connection_request(void)
{
  queue_event(g_app_ctx.event_queue,
              EVENT_TYPE_CONNECTION_REQUEST);
}
#endif

void app_trigger_switching_to_default_link(void)
{
  queue_event(g_app_ctx.event_queue,
              EVENT_TYPE_DEV_REGISTERED);
}

#if defined(SL_SID_APP_MSG_PRESENT)
void app_trigger_device_reset(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.rst_dev_ctx,
                                               ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_DEVICE_RESET);
}

void app_trigger_button_press(sl_sid_app_msg_dmp_soc_light_button_press_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.button_press_ctx,
                                               ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_BTN_PRESS);
}

void app_trigger_toggle_led(const sl_sid_app_msg_dmp_soc_light_toggle_led_ctx_t *ctx, enum toggle_led_source source)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.toggle_led_ctx,
                                               ctx);
  switch (source) {
    case TOGGLE_LED_SOURCE_BOARD:
      queue_event(g_app_ctx.event_queue, EVENT_TYPE_TOGGLE_LED_SOURCE_BOARD);
      break;
    case TOGGLE_LED_SOURCE_BLE:
      queue_event(g_app_ctx.event_queue, EVENT_TYPE_TOGGLE_LED_SOURCE_BLE);
      break;
    case TOGGLE_LED_SOURCE_CLOUD:
      queue_event(g_app_ctx.event_queue, EVENT_TYPE_TOGGLE_LED_SOURCE_CLOUD);
      break;
    default:
      SL_SID_LOG_APP_ERROR("invalid toggle led source: %d", (int)source);
      break;
  }
}

void app_trigger_ble_start_stop(sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.ble_start_stop_ctx,
                                               ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_BLE_START_STOP);
}

void app_trigger_update_counter(sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.update_counter_ctx,
                                               ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_COUNTER_UPDATE);
}

void app_trigger_time(sl_sid_app_msg_sid_time_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.time_ctx,
                                               ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_TIME);
}

void app_trigger_mtu(sl_sid_app_msg_sid_mtu_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.mtu_ctx,
                                               ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_MTU);
}

// Device management command class - reset device command callback
void sl_sid_app_msg_dev_mgmt_rst_dev_cb(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx)
{
  app_trigger_device_reset(ctx);
}

// Device management command class - button press command callback
void sl_sid_app_msg_dmp_soc_light_button_press_cb(sl_sid_app_msg_dmp_soc_light_button_press_ctx_t *ctx)
{
  app_trigger_button_press(ctx);
}

// Device management command class - toggle led command callback
void sl_sid_app_msg_dmp_soc_light_toggle_led_cb(sl_sid_app_msg_dmp_soc_light_toggle_led_ctx_t *ctx)
{
  app_trigger_toggle_led(ctx, TOGGLE_LED_SOURCE_CLOUD);
}

// DMP SOC Light command class - BLE start/stop command callback
void sl_sid_app_msg_dmp_soc_light_ble_start_stop_cb(sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx)
{
  app_trigger_ble_start_stop(ctx);
}

// DMP SOC Light command class - update counter command callback
void sl_sid_app_msg_dmp_soc_light_update_counter_cb(sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t *ctx)
{
  app_trigger_update_counter(ctx);
}

// Sidewalk command class - MTU command callback
void sl_sid_app_msg_sid_mtu_cb(sl_sid_app_msg_sid_mtu_ctx_t *ctx)
{
  app_trigger_mtu(ctx);
}

// Sidewalk command class - time command callback
void sl_sid_app_msg_sid_time_cb(sl_sid_app_msg_sid_time_ctx_t *ctx)
{
  app_trigger_time(ctx);
}
#endif

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
/*******************************************************************************
 * Button handler callback
 * @param[in] handle Button handler
 * @note This callback is called in the interrupt context
 ******************************************************************************/
void app_button_press_cb(uint8_t button, uint8_t duration)
{
#if defined(SL_SID_APP_MSG_PRESENT)
  g_app_ctx.app_msg.button_press_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  if (g_app_ctx.app_msg.button_press_ctx.hdl.processing) {
    SL_SID_LOG_APP_WARNING("button request already ongoing, dropped");
    goto send_response;
  }

  g_app_ctx.app_msg.button_press_ctx.hdl.processing = true;
  g_app_ctx.app_msg.button_press_ctx.param_send.button = button;
  g_app_ctx.app_msg.button_press_ctx.param_send.duration = duration;

  if (button == 0) { // PB0
    if ((duration == APP_BUTTON_PRESS_DURATION_SHORT)
        || (duration == APP_BUTTON_PRESS_DURATION_MEDIUM)) {
      #if (SL_SIMPLE_BUTTON_COUNT >= 2)
      sl_sid_app_msg_dmp_soc_light_toggle_led_ctx_t ctx = {
        .hdl.operation = SL_SID_APP_MSG_OP_NTFY
      };
      app_trigger_toggle_led(&ctx, TOGGLE_LED_SOURCE_BOARD);

      #else

      #if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
      sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t ctx = {
        .hdl.operation = SL_SID_APP_MSG_OP_NTFY
      };
      app_trigger_update_counter(&ctx);
      #endif
      #if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
      app_trigger_connect_and_send();
      #endif

      #endif
    } else { // long press
      // Start/Stop BLE stack
      sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t ctx = {
        .hdl.operation = SL_SID_APP_MSG_OP_NTFY
      };
      app_trigger_ble_start_stop(&ctx);
    }
#if (SL_SIMPLE_BUTTON_COUNT >= 2)
  } else if (button == 1) { // PB1
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t ctx = {
      .hdl.operation = SL_SID_APP_MSG_OP_NTFY
    };
    app_trigger_update_counter(&ctx);
#endif
#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
    app_trigger_connect_and_send();
#endif
#endif
  } else {
    SL_SID_LOG_APP_ERROR("BTN%d not exists", button);
    goto send_response;
  }

  g_app_ctx.app_msg.button_press_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  SL_SID_LOG_APP_INFO("BTN%d pressed, duration: %d", button, duration);

  send_response:

  queue_event(g_app_ctx.event_queue, EVENT_TYPE_BTN_PRESS_SEND_RESP);
#else
  (void)button;
  (void)duration;
#endif
}
#endif

/*******************************************************************************
 * Function to get the LED status
 *
 * @return LED status
 ******************************************************************************/
app_led_status_t app_get_led_status(void)
{
  return get_led_status(&g_app_ctx);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static int32_t init_and_start_link(app_context_t *app_ctx,
                                   struct sid_config *config,
                                   uint32_t link_mask)
{
  if (config->link_mask != link_mask) {
    sid_error_t ret = SID_ERROR_NONE;
    if (app_ctx->sidewalk_handle != NULL) {
      ret = sid_deinit(app_ctx->sidewalk_handle);
      if (ret != SID_ERROR_NONE) {
        SL_SID_LOG_APP_ERROR("sidewalk deinit failed, link mask: %x, error: %d",
                             link_mask, ret);
        goto error;
      }
      SL_SID_LOG_APP_INFO("sidewalk deinited, link mask: %x", link_mask);
    }

    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;
    // Initialize sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk init failed, link mask: %x, error: %d",
                           link_mask, ret);
      goto error;
    }
    SL_SID_LOG_APP_INFO("sidewalk inited, link mask: %x", link_mask);

    // Register sidewalk handler to the application context
    app_ctx->sidewalk_handle = sid_handle;
    // Start the sidewalk stack
    ret = sid_start(sid_handle, link_mask);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk start failed, link mask: %x, error: %d",
                           link_mask, ret);
      goto error;
    }
    SL_SID_LOG_APP_INFO("sidewalk started, link mask: %x", link_mask);
  }
  app_ctx->current_link_type = link_mask;

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
  btn_send_update_req = false;
#endif

  return 0;

  error:
  app_ctx->sidewalk_handle = NULL;
  config->link_mask = 0;
  return -1;
}

static uint32_t link_type_to_link_mask(uint8_t link_type)
{
  uint32_t ret = 0UL;

  switch (link_type) {
    case SL_SIDEWALK_LINK_BLE:
      ret = SID_LINK_TYPE_1;
      break;
    case SL_SIDEWALK_LINK_FSK:
      ret = SID_LINK_TYPE_2;
      break;
    case SL_SIDEWALK_LINK_CSS:
      ret = SID_LINK_TYPE_3;
      break;
    default:
      ret = SID_LINK_TYPE_ANY;
      break;
  }

  return ret;
}

static void queue_event(QueueHandle_t queue,
                        enum event_type event)
{
  if (queue == NULL) {
    return;
  }
  // Check if queue_event was called from ISR
  if ((bool)xPortIsInsideInterrupt()) {
    BaseType_t task_woken = pdFALSE;

    xQueueSendFromISR(queue, &event, &task_woken);
    portYIELD_FROM_ISR(task_woken);
  } else {
    xQueueSend(queue, &event, 0);
  }
}

static void on_sidewalk_event(bool in_isr,
                              void *context)
{
  app_context_t *app_ctx = NULL;
  (void)in_isr;

  if (context == NULL) {
    return;
  }

  app_ctx = (app_context_t *)context;

  // Issue sidewalk event to the queue
  queue_event(app_ctx->event_queue, EVENT_TYPE_SID_PROCESS_NEEDED);

}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *rcvd_sid_msg,
                                     void *context)
{
  (void)(context);

  if (msg_desc == NULL || rcvd_sid_msg == NULL) {
    return;
  }

  SL_SID_LOG_APP_INFO("downlink message received");
  SL_SID_LOG_APP_INFO(SID_RECEIVED_MSG_FORMAT_STR,
                      msg_desc->link_type,
                      msg_desc->id,
                      rcvd_sid_msg->size,
                      msg_desc->type,
                      msg_desc->msg_desc_attr.rx_attr.ack_requested,
                      msg_desc->msg_desc_attr.rx_attr.is_msg_ack,
                      msg_desc->msg_desc_attr.rx_attr.is_msg_duplicate,
                      msg_desc->msg_desc_attr.rx_attr.rssi,
                      msg_desc->msg_desc_attr.rx_attr.snr);
  if (rcvd_sid_msg->size != 0) {
    SL_SID_LOG_APP_INFO("received bytes:");
    SL_SID_LOG_APP_HEXDUMP_INFO((const void *)rcvd_sid_msg->data,
                                rcvd_sid_msg->size);
    if (sl_sidewalk_utils_is_data_ascii((const char *)rcvd_sid_msg->data,
                                        (uint16_t)rcvd_sid_msg->size)) {
      SL_SID_LOG_APP_INFO("received message: %.*s",
                          rcvd_sid_msg->size,(char *)rcvd_sid_msg->data);
    }
  }
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_st_t status = sl_sid_app_msg_handler(rcvd_sid_msg);
  if (status != SL_SID_APP_MSG_ERR_ST_SUCCESS) {
    SL_SID_LOG_APP_ERROR("message receive error, status: %d", status);
    return;
  }
#endif
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  (void)(context);

  if (msg_desc == NULL) {
    return;
  }

  SL_SID_LOG_APP_INFO("uplink message sent");
  SL_SID_LOG_APP_INFO("link type: %x, msg id: %u, msg type: %d",
                      msg_desc->link_type,
                      msg_desc->id,
                      msg_desc->type);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  (void)(context);

  if (msg_desc == NULL) {
    return;
  }

  SL_SID_LOG_APP_ERROR("uplink message send failed");
  SL_SID_LOG_APP_ERROR("link type: %x, msg id: %u, msg type: %d, error: %d",
                       msg_desc->link_type,
                       msg_desc->id,
                       msg_desc->type,
                       error);
}

static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  app_context_t *app_ctx = NULL;

  if (context == NULL || status == NULL) {
    return;
  }

  app_ctx = (app_context_t *)context;

  switch (status->state) {
    case SID_STATE_READY:
      app_ctx->state = STATE_SIDEWALK_READY;
      SL_SID_LOG_APP_INFO("sidewalk status ready");
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
      if (!fsk_profile_2_requested
          && (status->detail.link_status_mask & SID_LINK_TYPE_2)) {
        fsk_profile_2_requested = true;
        queue_event(app_ctx->event_queue, EVENT_TYPE_SET_FSK_PROFILE_2);
      }
#endif
      break;

    case SID_STATE_NOT_READY:
      app_ctx->state = STATE_SIDEWALK_NOT_READY;
      SL_SID_LOG_APP_INFO("sidewalk status not ready");
      break;

    case SID_STATE_ERROR:
      SL_SID_LOG_APP_ERROR("sidewalk status error: %d",
                           sid_get_error(app_ctx->sidewalk_handle));
      break;

    case SID_STATE_SECURE_CHANNEL_READY:
      app_ctx->state = STATE_SIDEWALK_SECURE_CONNECTION;
      SL_SID_LOG_APP_INFO("sidewalk secure channel ready");
      break;
  }

  if (status->detail.registration_status == SID_STATUS_REGISTERED) {
    app_trigger_switching_to_default_link();
  }

  SL_SID_LOG_APP_INFO("registration status: %u, time sync: %u, link: %lu",
                      status->detail.registration_status,
                      status->detail.time_sync_status,
                      status->detail.link_status_mask);

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
  if (btn_send_update_req && status->state == SID_STATE_READY) {
    btn_send_update_req = false;
    sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t ctx = {
      .hdl.operation = SL_SID_APP_MSG_OP_NTFY
    };
    app_trigger_update_counter(&ctx);
  }
#endif
}

static void on_sidewalk_factory_reset(void *context)
{
  (void)(context);
  SL_SID_LOG_APP_INFO("device factory reset");
  // This is the callback function of the factory reset and as the last step a reset is applied.
  NVIC_SystemReset();
}

#if defined(SL_SID_APP_MSG_PRESENT)
static void dev_reset_timer_cb(TimerHandle_t tmr_hdl)
{
  (void)tmr_hdl;

  if (g_app_ctx.app_msg.rst_dev_ctx.param_send.reset_type == SL_SID_APP_MSG_DEV_MGMT_VAL_RST_HARD) {
    sid_error_t ret = sid_set_factory_reset(g_app_ctx.sidewalk_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("factory reset failed: %d", ret);
    } else {
      SL_SID_LOG_APP_INFO("factory reset request accepted");
    }
  } else if (g_app_ctx.app_msg.rst_dev_ctx.param_send.reset_type == SL_SID_APP_MSG_DEV_MGMT_VAL_RST_SOFT) {
    SL_SID_LOG_APP_INFO("resetting device");
    NVIC_SystemReset();
  } else {
    SL_SID_LOG_APP_ERROR("unexpected reset type, reset type: %d",
                         g_app_ctx.app_msg.rst_dev_ctx.param_send.reset_type);
  }
}

static void exec_device_reset(app_context_t *app_ctx)
{
  sl_sid_app_msg_t app_msg;

  if (app_ctx == NULL) {
    return;
  }

  app_ctx->app_msg.rst_dev_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  if (xTimerIsTimerActive(app_ctx->device_reset_timer)) {
    SL_SID_LOG_APP_WARNING("reset device already in progress");
    goto send_response;
  }

  SL_SID_LOG_APP_INFO("reset device, type: %d, ms: %lu",
                      app_ctx->app_msg.rst_dev_ctx.param_send.reset_type,
                      app_ctx->app_msg.rst_dev_ctx.param_send.in_millisecs);

  uint32_t in_millisecs = app_ctx->app_msg.rst_dev_ctx.param_send.in_millisecs;
  if (in_millisecs == 0) {
    // 0 is invalid timeout value for sw timer
    // but it means immediate on the cloud side so we trigger timer cb manually
    in_millisecs = 1;
  }

  if (xTimerChangePeriod(app_ctx->device_reset_timer,
                         pdMS_TO_TICKS(in_millisecs),
                         0) == pdFALSE) {
    SL_SID_LOG_APP_ERROR("device reset timer period set failed");
    goto send_response;
  }

  if (xTimerStart(app_ctx->device_reset_timer, 0) == pdFALSE) {
    SL_SID_LOG_APP_ERROR("device reset timer start failed");
    goto send_response;
  }

  app_ctx->app_msg.rst_dev_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  send_response:

  sl_sid_app_msg_dev_mgmt_rst_dev_prepare_send(&app_ctx->app_msg.rst_dev_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}

static void exec_send_button_press_resp(app_context_t *app_ctx)
{
  sl_sid_app_msg_t app_msg;

  if (app_ctx == NULL) {
    return;
  }

  // the operation is already set for requests rather than real btn press
  if (!app_ctx->app_msg.button_press_ctx.is_emulation) {
    app_ctx->app_msg.button_press_ctx.hdl.operation = SL_SID_APP_MSG_OP_NTFY;
  }
  app_ctx->app_msg.button_press_ctx.is_emulation = false;

  sl_sid_app_msg_dmp_soc_light_button_press_prepare_send(&app_ctx->app_msg.button_press_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}

void sl_sidewalk_led_manager_led_state_changed(uint8_t led_id, sl_led_state_t new_led_state, sl_sidewalk_led_manager_source_t source)
{
  (void)led_id;
  sl_sid_app_msg_t app_msg;
  g_app_ctx.app_msg.toggle_led_ctx.param_send.state = new_led_state;
  g_app_ctx.app_msg.toggle_led_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  app_led_status_t led_status = {
    .value = g_app_ctx.app_msg.toggle_led_ctx.param_send.state,
    .source = (enum toggle_led_source)source
  };
  update_led_status(&g_app_ctx, led_status);

  SL_SID_LOG_APP_INFO("sending LED status, status: 0x%02x",
                      g_app_ctx.app_msg.toggle_led_ctx.param_send.state);

  if (source == SL_SIDEWALK_LED_MANAGER_SOURCE_BLE) {
    // Respond to the mobile application that the write request was successful
    app_bluetooth_toggle_led_write_response(true);
  }

  // Send indication to the mobile application that the LED state has changed
  app_bluetooth_update_led_status(g_app_ctx.app_msg.toggle_led_ctx.param_send.state, (enum toggle_led_source)source);

  sl_sid_app_msg_dmp_soc_light_toggle_led_prepare_send(&g_app_ctx.app_msg.toggle_led_ctx,
                                                       &app_msg);
  send_message(&g_app_ctx, &app_msg);
}

static void exec_ble_start_stop(app_context_t *app_ctx)
{
  if (app_ctx == NULL) {
    return;
  }

  app_ctx->app_msg.ble_start_stop_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  if (app_ctx->is_ble_running) {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    if (sl_bt_system_stop_bluetooth() != SL_STATUS_OK) {
      SL_SID_LOG_APP_ERROR("BLE stop failed");
      goto send_response;
    } else {
      #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
      sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(1));
      #endif
      app_ctx->is_ble_running = false;
      if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE,
                                 (const uint8_t *)&app_ctx->is_ble_running,
                                 sizeof(app_ctx->is_ble_running)) != 0) {
        SL_SID_LOG_APP_ERROR("NVM3 write failed");
      }
    }
#endif
#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
    app_bluetooth_stop_advertisement();
    #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
    sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(1));
    #endif
    app_ctx->is_ble_running = false;
    if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE,
                               (const uint8_t *)&app_ctx->is_ble_running,
                               sizeof(app_ctx->is_ble_running)) != 0) {
      SL_SID_LOG_APP_ERROR("NVM3 write failed");
    }
#endif
  } else {
#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
    if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
      SL_SID_LOG_APP_ERROR("BLE start failed");
      goto send_response;
    } else {
      #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
      sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
      #endif
      app_ctx->is_ble_running = true;
      if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE,
                                 (const uint8_t *)&app_ctx->is_ble_running,
                                 sizeof(app_ctx->is_ble_running)) != 0) {
        SL_SID_LOG_APP_ERROR("NVM3 write failed");
      }
    }
#endif
#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
    if (app_bluetooth_get_regular_ble_inited() == false) {
      app_bluetooth_init_and_start_advertisement();
    } else {
      app_bluetooth_start_advertisement();
    }
    #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
    sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
    #endif
    app_ctx->is_ble_running = true;
    if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE,
                               (const uint8_t *)&app_ctx->is_ble_running,
                               sizeof(app_ctx->is_ble_running)) != 0) {
      SL_SID_LOG_APP_ERROR("NVM3 write failed");
    }
#endif
  }

  app_ctx->app_msg.ble_start_stop_ctx.param_ack.optional = (uint16_t)app_ctx->is_ble_running;
  app_ctx->app_msg.ble_start_stop_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
  send_response:
#endif

  app_ctx->app_msg.ble_start_stop_ctx.param_send.state = (uint8_t)app_ctx->is_ble_running;
  SL_SID_LOG_APP_INFO("sending BLE status, status: 0x%02x",
                      app_ctx->app_msg.ble_start_stop_ctx.param_send.state);

  sl_sid_app_msg_t app_msg;

  sl_sid_app_msg_dmp_soc_light_ble_start_stop_prepare_send(&app_ctx->app_msg.ble_start_stop_ctx,
                                                           &app_msg);
  send_message(app_ctx, &app_msg);
}

static void exec_counter_update(app_context_t *app_ctx)
{
  sl_sid_app_msg_t app_msg;

  if (app_ctx == NULL) {
    return;
  }

  SL_SID_LOG_APP_INFO("sending counter update, counter: %d", app_ctx->counter);
  app_ctx->app_msg.update_counter_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;
  app_ctx->app_msg.update_counter_ctx.param_ack.optional = (uint16_t)app_ctx->counter;
  app_ctx->app_msg.update_counter_ctx.param_send.counter = app_ctx->counter;
  app_ctx->counter++;

  sl_sid_app_msg_dmp_soc_light_update_counter_prepare_send(&app_ctx->app_msg.update_counter_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}

static void exec_time(app_context_t *app_ctx)
{
  struct sid_timespec curr_time = SID_TIME_INFINITY;
  sl_sid_app_msg_t app_msg;

  if (app_ctx == NULL) {
    return;
  }

  app_ctx->app_msg.time_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  sid_error_t ret = sid_get_time(app_ctx->sidewalk_handle,
                                 SID_GET_GPS_TIME,
                                 &curr_time);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("get time failed: %d", ret);
    goto send_response;
  }

  SL_SID_LOG_APP_INFO("current time: %.02d.%.02d",
                      curr_time.tv_sec,
                      curr_time.tv_nsec);
  app_ctx->app_msg.time_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;
  app_ctx->app_msg.time_ctx.param_send.sec = curr_time.tv_sec;
  app_ctx->app_msg.time_ctx.param_send.nsec = curr_time.tv_nsec;

  send_response:

  sl_sid_app_msg_sid_time_prepare_send(&app_ctx->app_msg.time_ctx,
                                       &app_msg);
  send_message(app_ctx, &app_msg);
}

static void exec_mtu(app_context_t *app_ctx)
{
  uint32_t mtu = 0xFFFFFFFF;
  sl_sid_app_msg_t app_msg;

  if (app_ctx == NULL) {
    return;
  }

  app_ctx->app_msg.mtu_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  sid_error_t ret = sid_get_mtu(app_ctx->sidewalk_handle,
                                 app_ctx->app_msg.mtu_ctx.param_rcv.link_type,
                                 (size_t *)&mtu);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("get MTU failed: %d", ret);
    goto send_response;
  }

  SL_SID_LOG_APP_INFO("current MTU: %d", mtu);
  app_ctx->app_msg.mtu_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;
  app_ctx->app_msg.mtu_ctx.param_send.mtu = (uint16_t)mtu;

  send_response:

  sl_sid_app_msg_sid_mtu_prepare_send(&app_ctx->app_msg.mtu_ctx,
                                      &app_msg);
  send_message(app_ctx, &app_msg);
}

static void send_message(app_context_t *app_ctx, sl_sid_app_msg_t *app_msg)
{
  if (app_ctx == NULL || app_msg == NULL) {
    return;
  }

  if (app_ctx->state != STATE_SIDEWALK_READY
      && app_ctx->state != STATE_SIDEWALK_SECURE_CONNECTION) {
    SL_SID_LOG_APP_WARNING("sidewalk not ready yet");
    return;
  }

  // Convert application message into sidewalk message
  struct sid_msg send_sid_msg;
  sl_sid_app_msg_st_t status = sl_sid_app_msg_prepare_sid_msg(app_msg,
                                                              &send_sid_msg);
  if (status != SL_SID_APP_MSG_ERR_ST_SUCCESS) {
    SL_SID_LOG_APP_ERROR("message send error, status: %d", status);
    return;
  }

  // Send sidewalk message
  struct sid_msg_desc desc = {
    .type = SID_MSG_TYPE_NOTIFY,
    .link_type = SID_LINK_TYPE_ANY,
  };
  sid_error_t ret = sid_put_msg(app_ctx->sidewalk_handle,
                                &send_sid_msg,
                                &desc);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("send message failed, error: %d", ret);
    return;
  }

  SL_SID_LOG_APP_INFO("message queued");
  SL_SID_LOG_APP_INFO(SID_SEND_COUNTER_FORMAT_STR,
                      desc.link_type,
                      desc.id,
                      send_sid_msg.size,
                      desc.type,
                      desc.msg_desc_attr.tx_attr.request_ack,
                      desc.msg_desc_attr.tx_attr.ttl_in_seconds,
                      desc.msg_desc_attr.tx_attr.num_retries,
                      desc.msg_desc_attr.tx_attr.additional_attr);
  SL_SID_LOG_APP_HEXDUMP_INFO((const void *)send_sid_msg.data,
                              send_sid_msg.size);

  return;
}
#endif

#if defined(SL_SIDEWALK_DMP_FSK_SUPPORTED)
static void set_fsk_profile_2(app_context_t *app_ctx)
{
  struct sid_device_profile dev_cfg = {
    .unicast_params = {
      .device_profile_id = SID_LINK2_PROFILE_1
    }
  };

  if (app_ctx == NULL) {
    return;
  }

  sid_error_t ret = sid_option(app_ctx->sidewalk_handle,
                               SID_OPTION_900MHZ_GET_DEVICE_PROFILE,
                               &dev_cfg,
                               sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("FSK profile get failed, error: %d", ret);
    fsk_profile_2_requested = false;
    return;
  }

  if (dev_cfg.unicast_params.device_profile_id == SID_LINK2_PROFILE_2) {
    SL_SID_LOG_APP_INFO("FSK profile 2 already set");
    return;
  }

  dev_cfg.unicast_params.device_profile_id = SID_LINK2_PROFILE_2;

  ret = sid_option(app_ctx->sidewalk_handle,
                   SID_OPTION_900MHZ_SET_DEVICE_PROFILE,
                   &dev_cfg,
                   sizeof(dev_cfg));
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("FSK profile 2 set failed, error: %d", ret);
    fsk_profile_2_requested = false;
  } else {
    SL_SID_LOG_APP_INFO("FSK profile 2 set");
  }
}

static int32_t init_and_start_regular_ble(app_context_t *app_ctx)
{
  int32_t retVal = 0;

  if (app_ctx == NULL) {
    return -1;
  }

  // First startup, no valid nvm object is found, start BLE and save status to NVM
  if (sl_sidewalk_nvm3_is_valid_object(DMP_NVM3_KEY_BLE_STATE) != 1) {
    if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
      retVal = -1;
    }
    #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
    sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
    #endif
    app_ctx->is_ble_running = true;
    if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE,
                               (const uint8_t *)&app_ctx->is_ble_running,
                               sizeof(app_ctx->is_ble_running)) != 0) {
      SL_SID_LOG_APP_WARNING("NVM3 write failed");
    }
  } else {  // NVM object is found, read data
    if (sl_sidewalk_nvm3_read(DMP_NVM3_KEY_BLE_STATE,
                              (uint8_t *)&app_ctx->is_ble_running) != 0) {
      SL_SID_LOG_APP_WARNING("NVM3 read failed");
    } else {
      if (app_ctx->is_ble_running) {
        if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
          retVal = -1;
        }
        #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
        sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
        #endif
      }
    }
  }

  return retVal;
}
#endif

#if defined(SL_SIDEWALK_DMP_BLE_SUPPORTED)
static void init_and_start_regular_ble_advertisement(app_context_t *app_ctx)
{
  if (app_ctx == NULL) {
    return;
  }

  // First startup, no valid nvm object is found, start BLE advertisement and save status to NVM
  if (sl_sidewalk_nvm3_is_valid_object(DMP_NVM3_KEY_BLE_STATE) != 1) {
    app_bluetooth_init_and_start_advertisement();

    #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
    sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
    #endif

    app_ctx->is_ble_running = true;
    if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE,
                               (const uint8_t *)&app_ctx->is_ble_running,
                               sizeof(app_ctx->is_ble_running)) != 0) {
      SL_SID_LOG_APP_WARNING("NVM3 write failed");
    }
  } else {  // NVM object is found, read data
    if (sl_sidewalk_nvm3_read(DMP_NVM3_KEY_BLE_STATE,
                              (uint8_t *)&app_ctx->is_ble_running) != 0) {
      SL_SID_LOG_APP_WARNING("NVM3 read failed");
    } else {
      if (app_ctx->is_ble_running) {
        app_bluetooth_init_and_start_advertisement();

        #if defined(SL_CATALOG_SIMPLE_LED_PRESENT) && (SL_SIMPLE_LED_COUNT >= 2)
        sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
        #endif
      }
    }
  }
}
#endif

#if defined(SL_SID_APP_MSG_PRESENT)
static void update_led_status(app_context_t *app_ctx, app_led_status_t led_status)
{
  xSemaphoreTake(app_ctx->led_status_semaphore, portMAX_DELAY);
  app_ctx->led_status = led_status;
  xSemaphoreGive(app_ctx->led_status_semaphore);
}

static app_led_status_t get_led_status(app_context_t *app_ctx)
{
  app_led_status_t led_status;
  xSemaphoreTake(app_ctx->led_status_semaphore, portMAX_DELAY);
  led_status = app_ctx->led_status;
  xSemaphoreGive(app_ctx->led_status_semaphore);
  return led_status;
}
#endif
