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
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "app_process.h"
#include "app_init.h"
#include "app_assert.h"
#include "sid_api.h"
#include "sl_sidewalk_common_config.h"
#include "sl_sidewalk_cmd_executor.h"
#include "sl_sidewalk_sender.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_utils_config.h"
#include "sl_command_table.h"
#include "sl_sidewalk_log_app.h"

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

#if defined(SL_BLE_SUPPORTED)
#include "app_ble_config.h"
#include "sl_bt_api.h"
#endif

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "sl_simple_button_instances.h"
#endif

#if (defined(SL_CATALOG_LED0_PRESENT) || defined(SL_CATALOG_LED1_PRESENT))
#include "sl_simple_led_instances.h"
#endif

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
#include "sl_gpio.h"
#include "sl_rht_unidriver.h"
#include "sl_i2cspm_instances.h"
#include "sl_board_control_config.h"
#endif

#if defined(SL_TEMPERATURE_SENSOR_INTERNAL)
#include "tempdrv.h"
#endif

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
#include "sl_sidewalk_display.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Maximum number Queue elements
#define MSG_QUEUE_LEN       (20U)
#define UNUSED(x)           (void)(x)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Issue a queue event.
 *
 * @param[in] queue The queue handle which will be used ofr the event
 * @param[in] event The event to be sent
 ******************************************************************************/
static void queue_event(QueueHandle_t queue, enum event_type event);

/*******************************************************************************
 * Method for sending sidewalk events
 *
 * @param[in] in_isr If the event shall be handled from ISR context
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_event(bool in_isr, void *context);

/*******************************************************************************
 * Callback Method for receiving sidewalk messages
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] msg The received message
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);

/*******************************************************************************
 * Callback method for the case when a sidewalk message is sent
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);

/*******************************************************************************
 * Callback function if error happened during send operation
 *
 * @param[in] error The error type
 * @param[in] msg_desc Message descriptor
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context);

/*******************************************************************************
 * Callback Function to handle status changes in the Sidewalk context
 *
 * @param[in] status  new status
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

/*******************************************************************************
 * Callback function which is called from factory reset sidewalk event
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_factory_reset(void *context);

/*******************************************************************************
 * Function to convert link_type configuration to sidewalk stack link_mask
 *
 * @param[in] link_type  the link_type configuration to convert
 *
 * @returns link_mask  the corresponding link_mask enumeration
 ******************************************************************************/
static uint32_t link_type_to_link_mask(uint8_t link_type);

/*******************************************************************************
 * Function to do the main thread initialization
 *
 * @param[in] config The configuration for the sidewalk link
 *
 * @returns sid_error_t  SID_ERROR_GENERIC if the initialization failed
 *                       SID_ERROR_NONE if the initialization was successful
 ******************************************************************************/
static sid_error_t main_thread_init(struct sid_config *config);

/*******************************************************************************
 * Sending the message to the backend to request login credentials for OOB
 *
 * @return true
 * @return false
 ******************************************************************************/
static bool OOB_initiate_handshake_with_backend(void);

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
/*******************************************************************************
 * Timer callback to periodically measuring and reporting the temperature. It measures
 * the ambient temperature if sensor is available on the current target, else it measures
 * the core temperature of the MCU.
 *
 * @param pxTimer
 ******************************************************************************/
static void OOB_temperature_timer_cb(TimerHandle_t pxTimer);
#endif

#if (defined(SL_CATALOG_LED0_PRESENT) || defined(SL_CATALOG_LED1_PRESENT))
/*******************************************************************************
 * Timer callback to toggle LEDs periodically until the successful handshake process
 * is not finished and the device did not get the URL.
 *
 * @param pxTimer
 ******************************************************************************/
static void OOB_blinker_timer_cb(TimerHandle_t pxTimer);
#endif

/*******************************************************************************
 * Timer callback to periodically trigger message sending functionality. If there is
 * any message queued up it will try to send or retry to send a message in case
 * of a previous failure.
 *
 * @param pxTimer
 ******************************************************************************/
static void OOB_sender_timer_cb(TimerHandle_t pxTimer);

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
/*******************************************************************************
 * Timer callback to trigger periodic display update functionality. It redraws
 * the display if necessarry.
 *
 * @param pxTimer
 ******************************************************************************/
static void OOB_display_updater_timer_cb(TimerHandle_t pxTimer);
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
static sl_sidewalk_display_statistics_t statistics = {
  .is_registered  = false,
  .is_time_synced = false,
  .last_successful_tx_seq_num = 0,
  .link = 0
};

static TimerHandle_t display_updater_timer_hnd = NULL;
#endif

#if (defined(SL_CATALOG_LED0_PRESENT) || defined(SL_CATALOG_LED1_PRESENT))
static TimerHandle_t blinker_timer_hnd = NULL;
#endif

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static TimerHandle_t temperature_measure_timer_hnd = NULL;
#endif

static TimerHandle_t sender_timer_hnd = NULL;

// Queue for sending events
static QueueHandle_t g_event_queue;
// uint8_t type arguments
static uint8_t cli_arg_uint8_t;

static app_context_t application_context;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
static int32_t init_and_start_link(app_context_t *context,
                                   struct sid_config *config,
                                   uint32_t link_mask)
{
  if (context == NULL || config == NULL) {
    return -1;
  }

  if (config->link_mask != link_mask) {
    sid_error_t ret = SID_ERROR_NONE;
    if (context->sidewalk_handle != NULL) {
      ret = sid_deinit(context->sidewalk_handle);
      if (ret != SID_ERROR_NONE) {
        SL_SID_LOG_APP_ERROR("sidewalk deinit failed, link mask: %x, error: %d", (int)link_mask, (int)ret);
        goto error;
      }
      SL_SID_LOG_APP_INFO("sidewalk deinited, link mask: %x", (int)link_mask);
    }

    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;
    // Initialize sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk initialization failed, link mask: %x, error: %d", (int)link_mask, (int)ret);
      goto error;
    }
    SL_SID_LOG_APP_INFO("sidewalk inited, link mask: %x", (int)link_mask);

#if (defined(SL_SIDEWALK_COMMON_DEFAULT_LINK_CONNECTION_POLICY) && (SL_SIDEWALK_COMMON_DEFAULT_LINK_CONNECTION_POLICY == SID_LINK_CONNECTION_POLICY_MULTI_LINK_MANAGER)) \
    && defined(SL_SIDEWALK_COMMON_DEFAULT_MULTI_LINK_POLICY)
    enum sid_link_connection_policy link_conn_policy = SL_SIDEWALK_COMMON_DEFAULT_LINK_CONNECTION_POLICY;
    ret = sid_option(sid_handle, SID_OPTION_SET_LINK_CONNECTION_POLICY, &link_conn_policy, sizeof(enum sid_link_connection_policy));
    if (ret != SID_ERROR_NONE && ret != SID_ERROR_NOSUPPORT) {
      SL_SID_LOG_APP_ERROR("sidewalk connection policy set failed, error: %d", ret);
      goto error;
    } else if (ret == SID_ERROR_NOSUPPORT) {
      SL_SID_LOG_APP_WARNING("sidewalk connection policy set not supported on this platform");
    } else {
      SL_SID_LOG_APP_INFO("sidewalk connection policy set, policy: %d", (int)link_conn_policy);
    }

    enum sid_link_multi_link_policy multi_link_policy = SL_SIDEWALK_COMMON_DEFAULT_MULTI_LINK_POLICY;
    ret = sid_option(sid_handle, SID_OPTION_SET_LINK_POLICY_MULTI_LINK_POLICY, &multi_link_policy, sizeof(enum sid_link_multi_link_policy));
    if (ret != SID_ERROR_NONE && ret != SID_ERROR_NOSUPPORT) {
      SL_SID_LOG_APP_ERROR("sidewalk multi-link policy set failed, error: %d", ret);
      goto error;
    } else if (ret == SID_ERROR_NOSUPPORT) {
      SL_SID_LOG_APP_WARNING("sidewalk multi-link policy set not supported on this platform");
    } else {
      SL_SID_LOG_APP_INFO("sidewalk multi-link policy set, policy: %d", (int)multi_link_policy);
    }
#endif

    // Register sidewalk handler to the application context
    context->sidewalk_handle = sid_handle;
    // Start the sidewalk stack
    ret = sid_start(sid_handle, link_mask);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk start failed, link mask: %x, error: %d", (int)link_mask, (int)ret);
      goto error;
    }
    SL_SID_LOG_APP_INFO("sidewalk started, link mask: %x", (int)link_mask);
  }
  application_context.current_link_type = link_mask;

  return 0;

  error:
  context->sidewalk_handle = NULL;
  config->link_mask = 0U;
  return -1;
}

void oob_msg_receiver_thread(void *context)
{
  UNUSED(context);
  const TickType_t xDelay = 250 / portTICK_PERIOD_MS;

  while (1) {
    sl_sidewalk_cmd_executor_execute();
    vTaskDelay(xDelay);
  }

  // should never reach here
  sid_platform_deinit();
  vTaskDelete(NULL);
}

static uint32_t link_type_to_link_mask(uint8_t link_type)
{
  uint32_t ret;

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

static sid_error_t main_thread_init(struct sid_config *config)
{
  sid_error_t ret = SID_ERROR_NONE;

  if (!config) {
    return SID_ERROR_INVALID_ARGS;
  }

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

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  sl_gpio_t gpio = {
    .port = SL_BOARD_ENABLE_SENSOR_RHT_PORT,
    .pin = SL_BOARD_ENABLE_SENSOR_RHT_PIN,
  };
  sl_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin(&gpio);
  sl_rht_unidriver_init(sl_i2cspm_sensor);
#endif

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  config->sub_ghz_link_config = app_get_sub_ghz_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  config->link_config = app_get_ble_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  application_context.connection_request = false;
#endif

  // Queue creation for the sidewalk events
  g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(g_event_queue != NULL, "queue creation failed");

  // Initialize to not ready state
  application_context.state = STATE_SIDEWALK_NOT_READY;

  // Assign queue to the application context
  application_context.event_queue = g_event_queue;

  temperature_measure_timer_hnd = xTimerCreate("temperature_timer",
                                               pdMS_TO_TICKS(1000),
                                               pdTRUE,
                                               (void*)0,
                                               OOB_temperature_timer_cb);
  if (!temperature_measure_timer_hnd) {
    SL_SID_LOG_APP_ERROR("temperature measurement timer creation failed");
    ret = SID_ERROR_GENERIC;
    goto init_exit;
  }
  SL_SID_LOG_APP_INFO("temperature measurement timer created");

  blinker_timer_hnd = xTimerCreate("blinker_timer",
                                   pdMS_TO_TICKS(500),
                                   pdTRUE,
                                   (void*)0,
                                   OOB_blinker_timer_cb);
  if (!blinker_timer_hnd) {
    SL_SID_LOG_APP_ERROR("blinker timer creation failed");
    ret = SID_ERROR_GENERIC;
    goto init_exit;
  }
  SL_SID_LOG_APP_INFO("blinker timer started");

  sender_timer_hnd = xTimerCreate("sender_timer",
                                  pdMS_TO_TICKS(100),
                                  pdTRUE,
                                  (void*)0,
                                  OOB_sender_timer_cb);
  if (!sender_timer_hnd) {
    SL_SID_LOG_APP_ERROR("sender timer creation failed");
    ret = SID_ERROR_GENERIC;
    goto init_exit;
  }
  SL_SID_LOG_APP_INFO("sender timer created");

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  display_updater_timer_hnd = xTimerCreate("display_update_timer",
                                           pdMS_TO_TICKS(1000),
                                           pdTRUE,
                                           (void*)0,
                                           OOB_display_updater_timer_cb);
  if (!display_updater_timer_hnd) {
    SL_SID_LOG_APP_ERROR("display update timer creation failed");
    ret = SID_ERROR_GENERIC;
    goto init_exit;
  }
  SL_SID_LOG_APP_INFO("display update timer created");

  xTimerStart(display_updater_timer_hnd, 0);
  SL_SID_LOG_APP_INFO("display update timer started");
  sl_sidewalk_display_stats(&statistics);
#endif

  xTimerStart(blinker_timer_hnd, 0);
  SL_SID_LOG_APP_INFO("blinker timer started");
  xTimerStart(sender_timer_hnd, 0);
  SL_SID_LOG_APP_INFO("sender timer started");

  queue_event(g_event_queue, EVENT_TYPE_INITIATE_HANDSHAKE_WITH_BACKEND);

  if (init_and_start_link(&application_context, config, link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK)) != 0) {
    ret = SID_ERROR_GENERIC;
    goto init_exit;
  }

  init_exit:
  return ret;
}

void main_thread(void *context)
{
  // Creating application context
  (void)context;

  // Application context creation
  application_context.event_queue     = NULL;
  application_context.main_task       = NULL;
  application_context.sidewalk_handle = NULL;
  application_context.state           = STATE_INIT;

  // Register the callback functions and the context
  struct sid_event_callbacks event_callbacks =
  {
    .context           = &application_context,
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
    .dev_ch = {
      .type = SID_END_DEVICE_TYPE_STATIC,
      .power_type = SID_END_DEVICE_POWERED_BY_LINE_POWER_ONLY,
      .qualification_id = 0x0002,
    },
    .callbacks = &event_callbacks,
    .link_config = NULL,
    .sub_ghz_link_config = NULL,
  };

  if(main_thread_init(&config) != SID_ERROR_NONE)
  {
    SL_SID_LOG_APP_ERROR("main thread init failed");
    goto error;
  }

  while (1) {
    enum event_type event = EVENT_TYPE_INVALID;

    if (xQueueReceive(application_context.event_queue, &event, portMAX_DELAY) != pdTRUE) {
      continue;
    }
    // State machine for Sidewalk events
    switch (event) {
#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
      case EVENT_TYPE_DISPLAY:
        if (sl_sidewalk_display_update() == true) {
          SL_SID_LOG_APP_INFO("display updated");
        }
        break;
#endif
      case EVENT_TYPE_SIDEWALK:
        SL_SID_LOG_APP_DEBUG("sidewalk process event");
        sid_process(application_context.sidewalk_handle);
        break;

      case EVENT_TYPE_SEND:
        SL_SID_LOG_APP_DEBUG("send event");
        if (application_context.state == STATE_SIDEWALK_READY) {
          sl_sidewalk_sender_send(application_context.sidewalk_handle);
        } else {
          SL_SID_LOG_APP_WARNING("sidewalk not ready");
        }
        break;

      case EVENT_TYPE_INITIATE_HANDSHAKE_WITH_BACKEND:
        SL_SID_LOG_APP_INFO("initiate handshake event");
        OOB_initiate_handshake_with_backend();
        break;

      case EVENT_TYPE_REGISTERED:
        SL_SID_LOG_APP_INFO("device registered event");
        if (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE != SL_SIDEWALK_COMMON_REGISTRATION_LINK
            && init_and_start_link(&application_context,
                                   &config,
                                   link_type_to_link_mask(SL_SIDEWALK_LINK_TO_USE)) != 0) {
          goto error;
        }
        break;

#if defined(SL_BLE_SUPPORTED)
      case EVENT_TYPE_CONNECTION_REQUEST_ON:
        SL_SID_LOG_APP_INFO("BLE connection request on event");
        sid_ble_bcn_connection_request(application_context.sidewalk_handle, true);
        break;

      case EVENT_TYPE_CONNECTION_REQUEST_OFF:
        SL_SID_LOG_APP_INFO("BLE connection request off event");
        sid_ble_bcn_connection_request(application_context.sidewalk_handle, false);
        break;
#endif

      default:
        SL_SID_LOG_APP_ERROR("unexpected event: %d", (int)event);
        break;
    }
  }

  error:
  // If error happens deinit sidewalk
  if (application_context.sidewalk_handle != NULL) {
    sid_stop(application_context.sidewalk_handle, config.link_mask);
    sid_deinit(application_context.sidewalk_handle);
    application_context.sidewalk_handle = NULL;
  }
  SL_SID_LOG_APP_ERROR("unrecoverable error occurred");

  sid_platform_deinit();
  vTaskDelete(NULL);
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
/*******************************************************************************
 * Button handler callback
 * @param[in] handle Button handler
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  static bool previous_state = false;
  static uint8_t counter = 0;
  char buffer[20];
  memset(buffer, 0, sizeof(buffer));

  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
#if defined(SL_CATALOG_BTN0_PRESENT)
    if (&sl_button_btn0 == handle) {
      snprintf(buffer, sizeof(buffer), ":message=Counter %d", counter);
      sl_sidewalk_sender_queue_message(buffer, strlen(buffer), SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH);
      counter++;
    }
#endif

#if defined(SL_CATALOG_BTN1_PRESENT)
    if (&sl_button_btn1 == handle) {
      if (previous_state == true) {
        previous_state = false;
        char msg[] = ":button1=0";
        sl_sidewalk_sender_queue_message(msg, strlen(msg), SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH);
      } else {
        previous_state = true;
        char msg[] = ":button1=1";
        sl_sidewalk_sender_queue_message(msg, strlen(msg), SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH);
      }
    }
#endif
  }
}
#endif

void app_trigger_send(uint8_t len)
{
  cli_arg_uint8_t = len;
  queue_event(g_event_queue, EVENT_TYPE_SEND);

  SL_SID_LOG_APP_DEBUG("trigger send");
}

void app_trigger_switching_to_default_link(void)
{
  queue_event(g_event_queue, EVENT_TYPE_REGISTERED);
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
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
  UNUSED(in_isr);
  app_context_t *app_context = (app_context_t *)context;
  queue_event(app_context->event_queue, EVENT_TYPE_SIDEWALK);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context)
{
  UNUSED(context);

  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("           RECEIVED MESSAGE             ");
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("link type: %x, msg id: %u, msg size: %u, msg type: %d, ack requested: %d, is ack: %d, is duplicate: %d, rssi: %d, snr: %d",
                      msg_desc->link_type,
                      msg_desc->id,
                      msg->size,
                      (int)msg_desc->type,
                      msg_desc->msg_desc_attr.rx_attr.ack_requested,
                      msg_desc->msg_desc_attr.rx_attr.is_msg_ack,
                      msg_desc->msg_desc_attr.rx_attr.is_msg_duplicate,
                      msg_desc->msg_desc_attr.rx_attr.rssi,
                      msg_desc->msg_desc_attr.rx_attr.snr);
  if (msg->size != 0) {
    SL_SID_LOG_APP_INFO("%s", (char *) msg->data);
  }
  SL_SID_LOG_APP_INFO("########################################");

  sl_sidewalk_cmd_executor_recieve((char *) msg->data, msg->size);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  UNUSED(context);

  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("              MESSAGE SENT              ");
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("link type: %x, msg id: %u, msg type: %d",
                      msg_desc->link_type,
                      msg_desc->id,
                      (int)msg_desc->type);
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("########################################");

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  statistics.last_successful_tx_seq_num = msg_desc->id;
  sl_sidewalk_display_stats(&statistics);
#endif
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  UNUSED(context);

  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("                 ERROR                  ");
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_ERROR("link type: %x, msg id: %u, msg type: %d, error: %d",
                       msg_desc->link_type,
                       msg_desc->id,
                       (int)msg_desc->type,
                       (int)error);
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("########################################");
}

/*******************************************************************************
 * Sidewalk Status change handler
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  app_context_t *app_context = (app_context_t *)context;

  switch (status->state) {
    case SID_STATE_READY:
      app_context->state = STATE_SIDEWALK_READY;
      SL_SID_LOG_APP_INFO("sidewalk status ready");
      break;

    case SID_STATE_NOT_READY:
      app_context->state = STATE_SIDEWALK_NOT_READY;
      SL_SID_LOG_APP_INFO("sidewalk status not ready");
      break;

    case SID_STATE_ERROR:
      SL_SID_LOG_APP_ERROR("sidewalk status error, error: %d", (int)sid_get_error(app_context->sidewalk_handle));
      break;

    case SID_STATE_SECURE_CHANNEL_READY:
      app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
      SL_SID_LOG_APP_INFO("sidewalk secure channel ready");
      break;
  }

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  statistics.is_registered  = (bool)status->detail.registration_status;
  statistics.is_time_synced = (bool)status->detail.time_sync_status;
  statistics.link           = status->detail.link_status_mask;

  sl_sidewalk_display_stats(&statistics);
#endif

  if (status->detail.registration_status == SID_STATUS_REGISTERED) {
    app_trigger_switching_to_default_link();
  }

  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("        SIDEWALK STATUS CHANGED         ");
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("sidewalk status changed, status %d", (int)status->state);
  SL_SID_LOG_APP_INFO("registration status: %u, time sync: %u, link: %lu",
                      status->detail.registration_status,
                      status->detail.time_sync_status,
                      status->detail.link_status_mask);
  SL_SID_LOG_APP_INFO("########################################");
  SL_SID_LOG_APP_INFO("########################################");
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
  SL_SID_LOG_APP_INFO("device factory reset");
  // This is the callback function of the factory reset and as the last step a reset is applied.
  NVIC_SystemReset();
}

static bool OOB_initiate_handshake_with_backend(void)
{
  char msg_buffer[1 /* $ */ + 1 /* # */ + SL_SIDEWALK_UTILS_SMSN_STR_LENGTH];
  uint8_t buffer_index = 0U;
  memset(msg_buffer, 0, sizeof(msg_buffer));
  strcat(msg_buffer, "$#");

  buffer_index = (uint8_t)strlen(msg_buffer);

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  sl_sidewalk_display_message("Connecting...");
#endif

  sl_sidewalk_utils_get_smsn_as_str(&msg_buffer[buffer_index],
                                    SL_SIDEWALK_UTILS_SMSN_STR_LENGTH);

  return sl_sidewalk_sender_queue_message(msg_buffer,
                                          sizeof(msg_buffer),
                                          SL_SIDEWALK_SENDER_TYPE_PRIORITY_HIGH);
}

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
static void OOB_display_updater_timer_cb(TimerHandle_t pxTimer)
{
  UNUSED(pxTimer);

  if (g_event_queue != NULL) {
    queue_event(g_event_queue, EVENT_TYPE_DISPLAY);
  }
}
#endif

#if (defined(SL_CATALOG_LED0_PRESENT) || defined(SL_CATALOG_LED1_PRESENT))
static void OOB_blinker_timer_cb(TimerHandle_t pxTimer)
{
  UNUSED(pxTimer);

#if defined(SL_CATALOG_LED0_PRESENT)
  sl_simple_led_toggle(sl_led_led0.context);
#endif

#if defined(SL_CATALOG_LED1_PRESENT)
  sl_simple_led_toggle(sl_led_led1.context);
#endif
}
#endif

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static void OOB_temperature_timer_cb(TimerHandle_t pxTimer)
{
  UNUSED(pxTimer);

  int32_t temp_data;
  char number_buffer[8];
  memset(number_buffer, 0, sizeof(number_buffer));

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  uint32_t rh_data;
  sl_rht_unidriver_measure_rh_and_temp(&rh_data, &temp_data);
  snprintf(number_buffer, sizeof(number_buffer), "%.2f", (float)((float)temp_data / 1000.0f));
#elif defined(SL_TEMPERATURE_SENSOR_INTERNAL)
  temp_data = (int32_t)TEMPDRV_GetTemp();
  snprintf(number_buffer, sizeof(number_buffer), "%ld", temp_data);
#endif

  char msg[32];
  snprintf(msg, sizeof(msg), ":temperature=%s", number_buffer);
  sl_sidewalk_sender_queue_message(msg, strlen(msg), SL_SIDEWALK_SENDER_TYPE_PRIORITY_LOW);
}
#endif

void sl_sidewalk_cmd_executor_common_cb(sl_sidewalk_command_id_t command)
{
  switch (command) {
    case SIDEWALK_COMMAND_ID_ready:
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
      xTimerStart(temperature_measure_timer_hnd, 0);
#endif
      break;

    case SIDEWALK_COMMAND_ID_url:
#if (defined(SL_CATALOG_LED0_PRESENT) || defined(SL_CATALOG_LED1_PRESENT))
      xTimerStop(blinker_timer_hnd, 0);
#endif

#if defined(SL_CATALOG_LED0_PRESENT)
      sl_simple_led_turn_off(sl_led_led0.context);
#endif

#if defined(SL_CATALOG_LED1_PRESENT)
      sl_simple_led_turn_off(sl_led_led1.context);
#endif
      break;

    case SIDEWALK_COMMAND_ID_disconnect:
      queue_event(g_event_queue, EVENT_TYPE_INITIATE_HANDSHAKE_WITH_BACKEND);
#if (defined(SL_CATALOG_LED0_PRESENT) || defined(SL_CATALOG_LED1_PRESENT))
      xTimerStart(blinker_timer_hnd, 0);
#endif

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
      xTimerStop(temperature_measure_timer_hnd, 0);
#endif
      break;

    default:
      // Nothing to do
      break;
  }
}

static void OOB_sender_timer_cb(TimerHandle_t pxTimer)
{
  UNUSED(pxTimer);

  static TickType_t connection_request_start_time;
  struct sid_status current_status;

  sid_get_status(application_context.sidewalk_handle, &current_status);

  if ((current_status.detail.registration_status == 0) && (current_status.detail.time_sync_status == 0)) {
    if ((current_status.detail.link_status_mask == SID_LINK_TYPE_2) && (current_status.state == SID_STATE_READY)) {
      queue_event(g_event_queue, EVENT_TYPE_SEND);
#if defined(SL_BLE_SUPPORTED)
    } else if ((current_status.detail.link_status_mask == SID_LINK_TYPE_1) && (current_status.state == SID_STATE_READY)) {
      queue_event(g_event_queue, EVENT_TYPE_SEND);
    } else if (application_context.current_link_type == SID_LINK_TYPE_1) {
      if (application_context.connection_request == true) {
        if (xTaskGetTickCount() - connection_request_start_time >= pdMS_TO_TICKS(15000)) {
          queue_event(g_event_queue, EVENT_TYPE_CONNECTION_REQUEST_OFF);
          application_context.connection_request = false;
          connection_request_start_time = xTaskGetTickCount();
        }
      } else {
        if (xTaskGetTickCount() - connection_request_start_time >= pdMS_TO_TICKS(1500)) {
          queue_event(g_event_queue, EVENT_TYPE_CONNECTION_REQUEST_ON);
          application_context.connection_request = true;
          connection_request_start_time = xTaskGetTickCount();
        }
      }
#endif
    }
  }
}
