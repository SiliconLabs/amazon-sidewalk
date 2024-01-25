/***************************************************************************//**
 * @file
 * @brief app_process.c
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
#include <stdio.h>
#include <string.h>

#include "app_process.h"
#include "app_init.h"
#include "app_assert.h"
#include "app_log.h"
#include "sid_api.h"
#include "sl_bt_api.h"
#include "sl_sidewalk_common_config.h"
#include "sl_malloc.h"
#include "app_button_press.h"
#include "sl_sidewalk_nvm3_handler.h"

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "sl_simple_button_instances.h"
#endif

#if defined(SL_CATALOG_SIMPLE_LED_PRESENT)
#include "sl_simple_led_instances.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Maximum number Queue elements
#define MSG_QUEUE_LEN           (10U)
// Unused function parameter
#define UNUSED(x)               (void)(x)
// Key value for NVM data
enum app_nvm3_keys {
  DMP_NVM3_KEY_BLE_STATE = SLI_SID_NVM3_KEY_MIN_APP
};

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
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);

/*******************************************************************************
 * Callback for sidewalk message transmission
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);

/*******************************************************************************
 * Callback for sidewalk message transmission error
 *
 * @param[in] error The error type
 * @param[in] msg_desc Message descriptor
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context);

/*******************************************************************************
 * Callback for sidewalk status change
 *
 * @param[in] status New status
 * @param[in] context Application context
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

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

/*******************************************************************************
 * Function to init and start regular ble link
 *
 * @param[in] app_ctx Application context
 *
 * @return 0 on success, -1 otherwise
 ******************************************************************************/
static int32_t init_and_start_regular_ble(app_context_t *app_ctx);

/*******************************************************************************
 * Function to init and start sidewalk links
 *
 * @param[out] app_ctx Application context
 * @param[out] config Sidewalk configuration parameters
 * @param[in] link_mask Sidewalk stack link mask
 *
 * @return 0 on success, -1 otherwise
 ******************************************************************************/
static int32_t init_and_start_link(app_context_t *app_ctx, struct sid_config *config, uint32_t link_mask);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// Sidewalk application context
static app_context_t g_app_ctx;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void main_thread(void *context)
{
  (void)context;

  // Application context creation
  g_app_ctx.event_queue         = NULL;
  g_app_ctx.main_task           = NULL;
  g_app_ctx.sidewalk_handle     = NULL;
  g_app_ctx.state               = STATE_INIT;
  g_app_ctx.counter             = 0;
  g_app_ctx.is_ble_running      = false;
#if defined(SL_SID_APP_MSG_PRESENT)
  g_app_ctx.device_reset_timer  = NULL;
  memset(&g_app_ctx.app_msg, 0, sizeof(g_app_ctx.app_msg));
#endif

  // Register the callback functions and the context
  struct sid_event_callbacks event_callbacks =
  {
    .context           = &g_app_ctx,
    .on_event          = on_sidewalk_event,               // Called from ISR context
    .on_msg_received   = on_sidewalk_msg_received,        // Called from sid_process()
    .on_msg_sent       = on_sidewalk_msg_sent,            // Called from sid_process()
    .on_send_error     = on_sidewalk_send_error,          // Called from sid_process()
    .on_status_changed = on_sidewalk_status_changed,      // Called from sid_process()
    .on_factory_reset  = on_sidewalk_factory_reset,       // Called from sid_process()
  };

  // Set configuration parameters
  struct sid_config config =
  {
    .link_mask = 0,
    .callbacks   = &event_callbacks,
    .link_config = NULL,
    .sub_ghz_link_config = NULL,
  };

  // Queue creation for the sidewalk events
  g_app_ctx.event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(g_app_ctx.event_queue != NULL, "app: queue creation failed");

#if defined(SL_SID_APP_MSG_PRESENT)
  // Timer creation for the device reset
  g_app_ctx.device_reset_timer = xTimerCreate("tmr", 1 /* ticks */, pdFALSE /* auto-reload */, (void *)0, dev_reset_timer_cb);
  if (g_app_ctx.device_reset_timer == NULL) {
    app_log_error("app: device reset timer create failed");
    goto error;
  }
#endif

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  config.sub_ghz_link_config = app_get_sub_ghz_config();
#endif

  // Start regular BLE if needed
  if (init_and_start_regular_ble(&g_app_ctx) != 0) {
    goto error;
  }

  // Initialize to not ready state
  g_app_ctx.state = STATE_SIDEWALK_NOT_READY;

  // Assign queue to the application context
  g_app_ctx.event_queue = g_app_ctx.event_queue;

  // Initialize and start Sidewalk FSK
  if (init_and_start_link(&g_app_ctx, &config, link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK)) != 0) {
    goto error;
  }

  while (1) {
    enum event_type event;

    if (xQueueReceive(g_app_ctx.event_queue, &event, portMAX_DELAY) == pdTRUE) {
      // State machine for Sidewalk events
      switch (event) {
        case EVENT_TYPE_SID_PROCESS_NEEDED:
          sid_process(g_app_ctx.sidewalk_handle);
          break;

        case EVENT_TYPE_COUNTER_UPDATE:
          app_log_info("app: ctr update evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          exec_counter_update(&g_app_ctx);
#endif
          break;

        case EVENT_TYPE_TIME:
          app_log_info("app: get time evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          exec_time(&g_app_ctx);
#endif
          break;

        case EVENT_TYPE_MTU:
          app_log_info("app: get MTU evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          exec_mtu(&g_app_ctx);
#endif
          break;

        case EVENT_TYPE_DEVICE_RESET:
          app_log_info("app: device reset evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          exec_device_reset(&g_app_ctx);
#endif
          break;

        case EVENT_TYPE_DEV_REGISTERED:
          app_log_info("app: device registered evt");

          if (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE != SL_SIDEWALK_COMMON_REGISTRATION_LINK) {
            if (init_and_start_link(&g_app_ctx, &config, link_type_to_link_mask(SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE)) != 0) {
              goto error;
            }
          }
          break;

        case EVENT_TYPE_TOGGLE_LED:
          app_log_info("app: toggle led evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          sl_sidewalk_led_manager_toggle_led(g_app_ctx.app_msg.toggle_led_ctx.param_send.led);
#endif
          break;

        case EVENT_TYPE_BLE_START_STOP:
          app_log_info("app: ble start/stop evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          exec_ble_start_stop(&g_app_ctx);
#endif
          break;

        case EVENT_TYPE_BTN_PRESS:
          app_log_info("app: btn press evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          // if btn press is received as an RTOS event then it means that it's emulation
          // real btn press triggers btn press callback and not an RTOS event
          g_app_ctx.app_msg.button_press_ctx.is_emulation = true;
          // trigger btn press callback as if user pressed the button
          app_button_press_cb(
            g_app_ctx.app_msg.button_press_ctx.param_send.button,
            g_app_ctx.app_msg.button_press_ctx.param_send.duration);
#endif
          break;

        case EVENT_TYPE_BTN_PRESS_SEND_RESP:
          app_log_info("app: send btn press resp evt");

#if defined(SL_SID_APP_MSG_PRESENT)
          exec_send_button_press_resp(&g_app_ctx);
#endif
          break;

        default:
          app_log_error("app: unexpected evt: %d", (int)event);
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
  app_log_error("app: fatal error");

  sid_platform_deinit();
  vTaskDelete(NULL);
}

void app_trigger_switching_to_default_link(void)
{
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_DEV_REGISTERED);
}

#if defined(SL_SID_APP_MSG_PRESENT)
void app_trigger_device_reset(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.rst_dev_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_DEVICE_RESET);
}

void app_trigger_button_press(sl_sid_app_msg_dev_mgmt_button_press_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.button_press_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_BTN_PRESS);
}

void app_trigger_toggle_led(sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx)
{
  if (ctx->param_send.led == 1) {
    app_log_warning("app: led1 is reserved for another functionality");
    return;
  }

  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.toggle_led_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_TOGGLE_LED);
}

void app_trigger_ble_start_stop(sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.ble_start_stop_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_BLE_START_STOP);
}

void app_trigger_update_counter(sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.update_counter_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_COUNTER_UPDATE);
}

void app_trigger_time(sl_sid_app_msg_sid_time_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.time_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_TIME);
}

void app_trigger_mtu(sl_sid_app_msg_sid_mtu_ctx_t *ctx)
{
  APP_DROP_REQUEST_IF_ONGOING_OTHERWISE_ACCEPT(g_app_ctx.app_msg.mtu_ctx, ctx);
  queue_event(g_app_ctx.event_queue, EVENT_TYPE_MTU);
}

// Device management command class - reset device command callback
void sl_sid_app_msg_dev_mgmt_rst_dev_cb(sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t *ctx)
{
  app_trigger_device_reset(ctx);
}

// Device management command class - button press command callback
void sl_sid_app_msg_dev_mgmt_button_press_cb(sl_sid_app_msg_dev_mgmt_button_press_ctx_t *ctx)
{
  app_trigger_button_press(ctx);
}

// Device management command class - toggle led command callback
void sl_sid_app_msg_dev_mgmt_toggle_led_cb(sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t *ctx)
{
  app_trigger_toggle_led(ctx);
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
    app_log_warning("app: request already ongoing - drop");
    goto send_response;
  }

  g_app_ctx.app_msg.button_press_ctx.hdl.processing = true;
  g_app_ctx.app_msg.button_press_ctx.param_send.button = button;
  g_app_ctx.app_msg.button_press_ctx.param_send.duration = duration;

  if (button == 0) { // PB0
    if ((duration == APP_BUTTON_PRESS_DURATION_SHORT) || (duration == APP_BUTTON_PRESS_DURATION_MEDIUM)) {
      sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t ctx = { .param_send.led = 0, .hdl.operation = SL_SID_APP_MSG_OP_NTFY };
      app_trigger_toggle_led(&ctx);
    } else { // long press
      // Start/Stop BLE stack
      sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t ctx = { .hdl.operation = SL_SID_APP_MSG_OP_NTFY };
      app_trigger_ble_start_stop(&ctx);
    }
  } else if (button == 1) { // PB1
    sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t ctx = { .hdl.operation = SL_SID_APP_MSG_OP_NTFY };
    app_trigger_update_counter(&ctx);
  } else {
    app_log_error("app: btn%d does not exist", button);
    goto send_response;
  }

  g_app_ctx.app_msg.button_press_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  app_log_info("app: btn%d pressed (duration: %d)", button, duration);

  send_response:

  queue_event(g_app_ctx.event_queue, EVENT_TYPE_BTN_PRESS_SEND_RESP);
#else
  (void)button;
  (void)duration;
#endif
}
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static int32_t init_and_start_link(app_context_t *app_ctx, struct sid_config *config, uint32_t link_mask)
{
  if (config->link_mask != link_mask) {
    sid_error_t ret = SID_ERROR_NONE;
    if (app_ctx->sidewalk_handle != NULL) {
      ret = sid_deinit(app_ctx->sidewalk_handle);
      if (ret != SID_ERROR_NONE) {
        app_log_error("app: sid deinit failed, link:%x, err:%d", (int)link_mask, (int)ret);
        goto error;
      }
    }

    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;
    // Initialise sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
      app_log_error("app: sid init failed, link:%x, err:%d", (int)link_mask, (int)ret);
      goto error;
    }

    // Register sidewalk handler to the application context
    app_ctx->sidewalk_handle = sid_handle;
    // Start the sidewalk stack
    ret = sid_start(sid_handle, link_mask);
    if (ret != SID_ERROR_NONE) {
      app_log_error("app: sid start failed, link:%x, err:%d", (int)link_mask, (int)ret);
      goto error;
    }
  }
  app_ctx->current_link_type = link_mask;

  return 0;

  error:
  app_ctx->sidewalk_handle = NULL;
  config->link_mask = 0;
  return -1;
}

static uint32_t link_type_to_link_mask(uint8_t link_type)
{
  switch (link_type) {
    case SL_SIDEWALK_LINK_BLE:
      return SID_LINK_TYPE_1;
      break;
    case SL_SIDEWALK_LINK_FSK:
      return SID_LINK_TYPE_2;
      break;
    case SL_SIDEWALK_LINK_CSS:
      return SID_LINK_TYPE_3;
      break;
    default:
      return SID_LINK_TYPE_ANY;
      break;
  }
}

static void queue_event(QueueHandle_t queue,
                        enum event_type event)
{
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
  UNUSED(in_isr);
  app_context_t *app_ctx = (app_context_t *)context;
  // Issue sidewalk event to the queue
  queue_event(app_ctx->event_queue, EVENT_TYPE_SID_PROCESS_NEEDED);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *rcvd_sid_msg,
                                     void *context)
{
  UNUSED(context);
  app_log_info("app: sid msg rcvd (type: %d, id: %u, size: %u)", (int)msg_desc->type, msg_desc->id, rcvd_sid_msg->size);
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_st_t status = sl_sid_app_msg_handler(rcvd_sid_msg);
  if (status != SL_SID_APP_MSG_ERR_ST_SUCCESS) {
    app_log_error("app: app msg rcv err (status: %d)", status);
    return;
  }
#endif
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  UNUSED(context);
  app_log_info("app: msg sent (type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  UNUSED(context);
  app_log_error("app: send msg failed (type: %d, id: %u, err: %d)",
                (int)msg_desc->type, msg_desc->id, (int)error);
}

static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  app_context_t *app_ctx = (app_context_t *)context;

  app_log_info("app: sid status changed: %d", (int)status->state);

  switch (status->state) {
    case SID_STATE_READY:
      app_ctx->state = STATE_SIDEWALK_READY;
      break;

    case SID_STATE_NOT_READY:
      app_ctx->state = STATE_SIDEWALK_NOT_READY;
      break;

    case SID_STATE_ERROR:
      app_log_error("app: sid state err: %d", (int)sid_get_error(app_ctx->sidewalk_handle));
      break;

    case SID_STATE_SECURE_CHANNEL_READY:
      app_ctx->state = STATE_SIDEWALK_SECURE_CONNECTION;
      break;
  }

  if (status->detail.registration_status == SID_STATUS_REGISTERED) {
    app_trigger_switching_to_default_link();
  }

  app_log_info("app: REG: %u, TIME: %u, LINK: %lu",
               status->detail.registration_status,
               status->detail.time_sync_status,
               status->detail.link_status_mask);
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
  app_log_info("app: factory reset notif rcvd");
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
      app_log_error("app: factory reset notif failed");
    } else {
      app_log_info("app: wait to proceed with factory reset");
    }
  } else if (g_app_ctx.app_msg.rst_dev_ctx.param_send.reset_type == SL_SID_APP_MSG_DEV_MGMT_VAL_RST_SOFT) {
    app_log_info("app: resetting device");
    NVIC_SystemReset();
  } else {
    app_log_error("app: unexpected reset type: %d", g_app_ctx.app_msg.rst_dev_ctx.param_send.reset_type);
  }
}
#endif

#if defined(SL_SID_APP_MSG_PRESENT)
static void exec_device_reset(app_context_t *app_ctx)
{
  sl_sid_app_msg_t app_msg;

  app_ctx->app_msg.rst_dev_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  if (xTimerIsTimerActive(app_ctx->device_reset_timer)) {
    app_log_warning("app: reset device already in progress");
    goto send_response;
  }

  app_log_info("app: reset device (type: %d) in %lu ms",
               app_ctx->app_msg.rst_dev_ctx.param_send.reset_type,
               app_ctx->app_msg.rst_dev_ctx.param_send.in_millisecs);

  uint32_t in_millisecs = app_ctx->app_msg.rst_dev_ctx.param_send.in_millisecs;
  if (in_millisecs == 0) {
    // 0 is invalid timeout value for sw timer but it means immediate on the cloud side so we trigger timer cb manually
    in_millisecs = 1;
  }

  if (xTimerChangePeriod(app_ctx->device_reset_timer, pdMS_TO_TICKS(in_millisecs), 0) == pdFALSE) {
    app_log_error("app: device reset timer period set failed");
    goto send_response;
  }

  if (xTimerStart(app_ctx->device_reset_timer, 0) == pdFALSE) {
    app_log_error("app: device reset timer start failed");
    goto send_response;
  }

  app_ctx->app_msg.rst_dev_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  send_response:

  sl_sid_app_msg_dev_mgmt_rst_dev_prepare_send(&app_ctx->app_msg.rst_dev_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}
#endif

#if defined(SL_SID_APP_MSG_PRESENT)
static void exec_send_button_press_resp(app_context_t *app_ctx)
{
  sl_sid_app_msg_t app_msg;

  // the operation is already set for requests rather than real btn press
  if (!app_ctx->app_msg.button_press_ctx.is_emulation) {
    app_ctx->app_msg.button_press_ctx.hdl.operation = SL_SID_APP_MSG_OP_NTFY;
  }
  app_ctx->app_msg.button_press_ctx.is_emulation = false;

  sl_sid_app_msg_dev_mgmt_button_press_prepare_send(&app_ctx->app_msg.button_press_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}

void sl_sidewalk_led_manager_led_state_changed(uint8_t led_id, sl_led_state_t new_led_state)
{
  sl_sid_app_msg_t app_msg;

  g_app_ctx.app_msg.toggle_led_ctx.param_send.led = led_id;
  g_app_ctx.app_msg.toggle_led_ctx.param_send.state = new_led_state;
  g_app_ctx.app_msg.toggle_led_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  app_log_info("app: sending led status: 0x%02x", g_app_ctx.app_msg.toggle_led_ctx.param_send.state);

  // Bluetooth update
  app_bluetooth_update_led_status(g_app_ctx.app_msg.toggle_led_ctx.param_send.state);

  sl_sid_app_msg_dev_mgmt_toggle_led_prepare_send(&g_app_ctx.app_msg.toggle_led_ctx, &app_msg);
  send_message(&g_app_ctx, &app_msg);
}

static void exec_ble_start_stop(app_context_t *app_ctx)
{
  app_ctx->app_msg.ble_start_stop_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  if (app_ctx->is_ble_running) {
    if (sl_bt_system_stop_bluetooth() != SL_STATUS_OK) {
      app_log_error("app: BLE stop failed");
      goto send_response;
    } else {
      #if defined(SL_CATALOG_SIMPLE_LED_PRESENT)
      sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(1));
      #endif
      app_ctx->is_ble_running = false;
      if ((sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE, (const uint8_t *)&app_ctx->is_ble_running, sizeof(app_ctx->is_ble_running)) != 0)) {
        app_log_error("app: NVM data write failed");
      }
    }
  } else {
    if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
      app_log_error("app: BLE start failed");
      goto send_response;
    } else {
      #if defined(SL_CATALOG_SIMPLE_LED_PRESENT)
      sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
      #endif
      app_ctx->is_ble_running = true;
      if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE, (const uint8_t *)&app_ctx->is_ble_running, sizeof(app_ctx->is_ble_running)) != 0) {
        app_log_error("app: NVM data write failed");
      }
    }
  }

  app_ctx->app_msg.ble_start_stop_ctx.param_ack.optional = (uint16_t)app_ctx->is_ble_running;
  app_ctx->app_msg.ble_start_stop_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;

  send_response:

  app_log_info("app: sending ble status: 0x%02x", app_ctx->app_msg.ble_start_stop_ctx.param_send.state);

  sl_sid_app_msg_t app_msg;

  sl_sid_app_msg_dmp_soc_light_ble_start_stop_prepare_send(&app_ctx->app_msg.ble_start_stop_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}

static void exec_counter_update(app_context_t *app_ctx)
{
  sl_sid_app_msg_t app_msg;

  app_log_info("app: sending ctr update: %d", app_ctx->counter);
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

  app_ctx->app_msg.time_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  sid_error_t ret = sid_get_time(app_ctx->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: get time failed: %d", ret);
    goto send_response;
  }

  app_log_info("app: curr time: %lu.%lu", curr_time.tv_sec, curr_time.tv_nsec);
  app_ctx->app_msg.time_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;
  app_ctx->app_msg.time_ctx.param_send.sec = curr_time.tv_sec;
  app_ctx->app_msg.time_ctx.param_send.nsec = curr_time.tv_nsec;

  send_response:

  sl_sid_app_msg_sid_time_prepare_send(&app_ctx->app_msg.time_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}

static void exec_mtu(app_context_t *app_ctx)
{
  uint32_t mtu = 0xFFFFFFFF;
  sl_sid_app_msg_t app_msg;

  app_ctx->app_msg.mtu_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_NACK_VAL;

  sid_error_t ret = sid_get_mtu(app_ctx->sidewalk_handle, app_ctx->app_msg.mtu_ctx.param_rcv.link_type, (size_t *)&mtu);
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: get MTU failed: %d (link: %d)", ret, app_ctx->app_msg.mtu_ctx.param_rcv.link_type);
    goto send_response;
  }

  app_log_info("app: curr MTU: %lu", mtu);
  app_ctx->app_msg.mtu_ctx.param_ack.ack_nack = SL_SID_APP_MSG_APP_ACK_VAL;
  app_ctx->app_msg.mtu_ctx.param_send.mtu = (uint16_t)mtu;

  send_response:

  sl_sid_app_msg_sid_mtu_prepare_send(&app_ctx->app_msg.mtu_ctx, &app_msg);
  send_message(app_ctx, &app_msg);
}
#endif

static int32_t init_and_start_regular_ble(app_context_t *app_ctx)
{
  int32_t retVal = 0;

  // First startup, no valid nvm object is found, start BLE and save status to NVM
  if (sl_sidewalk_nvm3_get_valid_object_number() == 0) {
    if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
      retVal = -1;
    }
    #if defined(SL_CATALOG_SIMPLE_LED_PRESENT)
    sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
    #endif
    app_ctx->is_ble_running = true;
    if (sl_sidewalk_nvm3_write(DMP_NVM3_KEY_BLE_STATE, (const uint8_t *)&app_ctx->is_ble_running, sizeof(app_ctx->is_ble_running)) != 0) {
      app_log_warning("app: NVM data write failed");
    }
  } else {  // NVM object is found, read data
    if (sl_sidewalk_nvm3_read(DMP_NVM3_KEY_BLE_STATE, (uint8_t *)&app_ctx->is_ble_running) != 0) {
      app_log_warning("app: NVM data read failed");
    } else {
      if (app_ctx->is_ble_running) {
        if (sl_bt_system_start_bluetooth() != SL_STATUS_OK) {
          retVal = -1;
        }
        #if defined(SL_CATALOG_SIMPLE_LED_PRESENT)
        sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(1));
        #endif
      }
    }
  }

  return retVal;
}

#if defined(SL_SID_APP_MSG_PRESENT)
static void send_message(app_context_t *app_ctx, sl_sid_app_msg_t *app_msg)
{
  if (app_ctx->state != STATE_SIDEWALK_READY && app_ctx->state != STATE_SIDEWALK_SECURE_CONNECTION) {
    app_log_warning("app: msg cant be sent as sid is not ready yet");
    return;
  }

  // Convert application message into sidewalk message
  struct sid_msg send_sid_msg;
  sl_sid_app_msg_st_t status = sl_sid_app_msg_prepare_sid_msg(app_msg, &send_sid_msg);
  if (status != SL_SID_APP_MSG_ERR_ST_SUCCESS) {
    app_log_error("app: app msg send error (status: %d)", status);
    return;
  }

  // Send sidewalk message
  struct sid_msg_desc desc = {
    .type = SID_MSG_TYPE_NOTIFY,
    .link_type = SID_LINK_TYPE_ANY,
  };
  sid_error_t ret = sid_put_msg(app_ctx->sidewalk_handle, &send_sid_msg, &desc);
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: queueing data failed: %d", (int)ret);
    return;
  }

  app_log_info("app: queued data msg id: %u", desc.id);
  app_log_hexdump_info(send_sid_msg.data, send_sid_msg.size);

  return;
}
#endif
