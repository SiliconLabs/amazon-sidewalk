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

#include "sl_component_catalog.h"
#if defined(SL_CATALOG_APP_BUTTON_PRESS_PRESENT) || defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "app_button_press.h"
#endif
#include "app_init.h"
#include "app_process.h"
#include "app_assert.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_utils.h"
#include "sid_api.h"
#include "sl_sidewalk_common_config.h"
#if defined(SL_BOARD_SUPPORT)
#include "sl_sidewalk_board_support.h"
#endif

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
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Maximum number Queue elements
#define MSG_QUEUE_LEN       (10U)

#define UNUSED(x) (void)(x)
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
 * Function to send updated counter
 *
 * @param[in] app_context The context which is applicable for the current application
 ******************************************************************************/
static void send_counter_update(app_context_t *app_context);

/*******************************************************************************
 * Function to get time
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void get_time(app_context_t *context);

/*******************************************************************************
 * Function to get current MTU
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void get_mtu(app_context_t *context);

/*******************************************************************************
 * Function to execute Factory reset
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void factory_reset(app_context_t *context);

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
 * Function to switch between available links
 *
 * @param[out] app_context The context which is applicable for the current application
 * @param[out] config The configuration parameters
 *
 * @returns #true           on success
 * @returns #false          on failure
 ******************************************************************************/
static bool link_switch(app_context_t *app_context, struct sid_config *config);

/*******************************************************************************
 * Function to convert link_type configuration to sidewalk stack link_mask
 *
 * @param[in] link_type  the link_type configuration to convert
 *
 * @returns link_mask  the corresponding link_mask enumeration
 ******************************************************************************/
static uint32_t link_type_to_link_mask(uint8_t link_type);

#if defined(SL_BLE_SUPPORTED)
/*******************************************************************************
 * Function to trigger the connection request towards GW
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void toggle_connection_request(app_context_t *context);
#endif
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// Queue for sending events
static QueueHandle_t g_event_queue;
#if defined(SL_BLE_SUPPORTED)
// button send update request
static bool button_send_update_req;
#endif

static app_context_t application_context;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
static int32_t init_and_start_link(app_context_t *context, struct sid_config *config, uint32_t link_mask)
{
  if (config->link_mask != link_mask) {
    sid_error_t ret = SID_ERROR_NONE;
    if (context->sidewalk_handle != NULL) {
      ret = sid_deinit(context->sidewalk_handle);
      if (ret != SID_ERROR_NONE) {
        SL_SID_LOG_APP_ERROR("sidewalk deinitialization failed, link mask: %x, error: %d", (int)link_mask, (int)ret);
        goto error;
      }
      SL_SID_LOG_APP_INFO("sidewalk deinitializated, link mask: %x", (int)link_mask);
    }

    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;
    // Initialise sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk initialization failed, link mask: %x, error: %d", (int)link_mask, (int)ret);
      goto error;
    }
    SL_SID_LOG_APP_INFO("sidewalk initializated, link mask: %x", (int)link_mask);

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
#if defined(SL_BLE_SUPPORTED)
  button_send_update_req = false;
#endif

  return 0;

  error:
  context->sidewalk_handle = NULL;
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

#if defined(SL_BOARD_SUPPORT)
  sl_sidewalk_board_support_init();
  SL_SID_LOG_APP_INFO("board support initialized");
#endif

#if defined(SL_BOARD_SUPPORT) && (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  sl_sidewalk_start_temperature_timer();
  SL_SID_LOG_APP_INFO("temperature measurement timer started");
#endif

  // Queue creation for the sidewalk events
  g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(g_event_queue != NULL, "queue creation failed");

#if defined(SL_BLE_SUPPORTED)
  SL_SID_LOG_APP_INFO("BLE link supported");
#endif

#if defined(SL_FSK_SUPPORTED)
  SL_SID_LOG_APP_INFO("FSK link supported");
#endif

#if defined(SL_CSS_SUPPORTED)
  SL_SID_LOG_APP_INFO("CSS link supported");
#endif

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  config.sub_ghz_link_config = app_get_sub_ghz_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  config.link_config = app_get_ble_config();
  button_send_update_req = false;
#endif

#if defined(SL_BLE_SUPPORTED)
  application_context.connection_request = false;
#endif

  // Initialize to not ready state
  application_context.state = STATE_SIDEWALK_NOT_READY;

  // Assign queue to the application context
  application_context.event_queue = g_event_queue;

  if (init_and_start_link(&application_context, &config, link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK)) != 0) {
    goto error;
  }

  SL_SID_LOG_APP_INFO("main task started");

  while (1) {
    enum event_type event = EVENT_TYPE_INVALID;

    if (xQueueReceive(application_context.event_queue, &event, portMAX_DELAY) == pdTRUE) {
      // State machine for Sidewalk events
      switch (event) {
        case EVENT_TYPE_SIDEWALK:
          SL_SID_LOG_APP_DEBUG("sidewalk process event");
          sid_process(application_context.sidewalk_handle);
          break;

        case EVENT_TYPE_SEND_COUNTER_UPDATE:
          SL_SID_LOG_APP_INFO("counter update event");

          if (application_context.state == STATE_SIDEWALK_READY) {
            send_counter_update(&application_context);
          } else {
            SL_SID_LOG_APP_WARNING("sidewalk not ready yet");
          }
          break;

        case EVENT_TYPE_GET_TIME:
          SL_SID_LOG_APP_INFO("get time event");

          get_time(&application_context);
          break;

        case EVENT_TYPE_GET_MTU:
          SL_SID_LOG_APP_INFO("get MTU event");

          get_mtu(&application_context);
          break;

        case EVENT_TYPE_FACTORY_RESET:
          SL_SID_LOG_APP_INFO("factory reset event");

          factory_reset(&application_context);
          break;

        case EVENT_TYPE_LINK_SWITCH:
          SL_SID_LOG_APP_INFO("link switch event");

          if (link_switch(&application_context, &config) != true) {
            goto error;
          }
          break;

        case EVENT_TYPE_REGISTERED:
          SL_SID_LOG_APP_INFO("device registered event");

          if (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE != SL_SIDEWALK_COMMON_REGISTRATION_LINK) {
            if (init_and_start_link(&application_context, &config, link_type_to_link_mask(SL_SIDEWALK_LINK_TO_USE)) != 0) {
              goto error;
            }
          }
          break;

#if defined(SL_BLE_SUPPORTED)
        case EVENT_TYPE_CONNECTION_REQUEST:
          SL_SID_LOG_APP_INFO("BLE connection request event");

          toggle_connection_request(&application_context);
          break;
#endif

        default:
          SL_SID_LOG_APP_ERROR("unexpected event: %d", (int)event);
          break;
      }
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
 * @note This callback is called in the interrupt context
 ******************************************************************************/
void app_button_press_cb(uint8_t button, uint8_t duration)
{
  SL_SID_LOG_APP_INFO("button pressed, button: %d, duration: %d", button, duration);
  if (button == 0) { // PB0
#if !defined(SL_CATALOG_BTN1_PRESENT) // KG100S
    if (duration != APP_BUTTON_PRESS_DURATION_SHORT) { // long press
      app_trigger_link_switch();
    } else { // short press
      app_trigger_connect_and_send();
    }
#else // All others target others than KG100S
    (void)duration;
    app_trigger_link_switch();
#endif  // !defined(SL_CATALOG_BTN1_PRESENT)
  } else { // PB1
#if !defined(SL_CATALOG_BTN1_PRESENT) //KG100S
    SL_SID_LOG_APP_ERROR("kg100s BTN1 not configured");
#else
    app_trigger_connect_and_send();
#endif // !defined(SL_CATALOG_BTN1_PRESENT)
  }
}
#endif

void app_trigger_connect_and_send(void)
{
  if (application_context.current_link_type & SID_LINK_TYPE_1) { // BLE
#if defined(SL_BLE_SUPPORTED)
    if (application_context.state != STATE_SIDEWALK_READY) {
      if (!button_send_update_req) {
        button_send_update_req = true;
        app_trigger_connection_request();
      } else {
        SL_SID_LOG_APP_WARNING("connection request already in progress");
      }
    } else {
      app_trigger_send_counter_update();
    }
#endif // defined(SL_BLE_SUPPORTED)
  } else { // FSK or CSS
    app_trigger_send_counter_update();
  }
}

#if defined(SL_BLE_SUPPORTED)
static void toggle_connection_request(app_context_t *context)
{
  if (context->state == STATE_SIDEWALK_READY) {
    SL_SID_LOG_APP_WARNING("BLE connection is already established");
  } else {
    context->connection_request = true;

    sid_error_t ret = sid_ble_bcn_connection_request(context->sidewalk_handle, context->connection_request);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("BLE connection request failed, error: %d", (int)ret);
      return;
    }

    SL_SID_LOG_APP_INFO("BLE connection request set");
  }
}
#endif

void app_trigger_switching_to_default_link(void)
{
  queue_event(g_event_queue, EVENT_TYPE_REGISTERED);
}

void app_trigger_link_switch(void)
{
  queue_event(g_event_queue, EVENT_TYPE_LINK_SWITCH);
}

void app_trigger_send_counter_update(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SEND_COUNTER_UPDATE);
}

void app_trigger_factory_reset(void)
{
  queue_event(g_event_queue, EVENT_TYPE_FACTORY_RESET);
}

void app_trigger_get_time(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_TIME);
}

void app_trigger_get_mtu(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_MTU);
}

#if defined(SL_BLE_SUPPORTED)
void app_trigger_connection_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_CONNECTION_REQUEST);
}
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void queue_event(QueueHandle_t queue,
                        enum event_type event)
{
  if(queue == NULL)
  {
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
  // Issue sidewalk event to the queue
  queue_event(app_context->event_queue, EVENT_TYPE_SIDEWALK);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context)
{
  UNUSED(context);
  SL_SID_LOG_APP_INFO("downlink message received");
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
    SL_SID_LOG_APP_INFO("received bytes:");
    SL_SID_LOG_APP_HEXDUMP_INFO((const void *)msg->data, msg->size);
    if (sl_sidewalk_utils_is_data_ascii((const char *)msg->data, msg->size)) {
      SL_SID_LOG_APP_INFO("received message: %.*s", msg->size, (char *)msg->data);
    }
  }
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  UNUSED(context);
  SL_SID_LOG_APP_INFO("uplink message sent");
  SL_SID_LOG_APP_INFO("link type: %x, msg id: %u, msg type: %d",
                      msg_desc->link_type,
                      msg_desc->id,
                      (int)msg_desc->type);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  UNUSED(context);
  SL_SID_LOG_APP_ERROR("uplink message send failed");
  SL_SID_LOG_APP_ERROR("link type: %x, msg id: %u, msg type: %d, error: %d",
                       msg_desc->link_type,
                       msg_desc->id,
                       (int)msg_desc->type,
                       (int)error);
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

  if (status->detail.registration_status == SID_STATUS_REGISTERED) {
    app_trigger_switching_to_default_link();
  }

  SL_SID_LOG_APP_INFO("registration status: %u, time sync: %u, link: %lu",
                      status->detail.registration_status,
                      status->detail.time_sync_status,
                      status->detail.link_status_mask);

#if defined(SL_BLE_SUPPORTED)
  if (button_send_update_req && status->state == SID_STATE_READY) {
    button_send_update_req = false;
    app_trigger_send_counter_update();
  }
#endif
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
  SL_SID_LOG_APP_INFO("device factory reset");
  // This is the callback function of the factory reset and as the last step a reset is applied.
  NVIC_SystemReset();
}

/*******************************************************************************
 * Function that returns the next available link in the order BLE -> FSK -> CSS
 * @param[in] current_link Current link
 * @return Next available link
 * @note Returns the same link if there is no available link to switch
 ******************************************************************************/
static enum sid_link_type get_next_link(enum sid_link_type current_link)
{
  // BLE -> FSK -> CSS
  if (current_link == SID_LINK_TYPE_1) {
#if defined(SL_FSK_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to FSK link");
    return SID_LINK_TYPE_2;
#elif defined(SL_CSS_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to CSS link");
    return SID_LINK_TYPE_3;
#endif
    return SID_LINK_TYPE_1;
  } else if (current_link == SID_LINK_TYPE_2) {
#if defined(SL_CSS_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to CSS link");
    return SID_LINK_TYPE_3;
#elif defined(SL_BLE_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to BLE link");
    return SID_LINK_TYPE_1;
#endif
    return SID_LINK_TYPE_2;
  } else { // (current_link == SID_LINK_TYPE_3)
#if defined(SL_BLE_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to BLE link");
    return SID_LINK_TYPE_1;
#elif defined(SL_FSK_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to FSK link");
    return SID_LINK_TYPE_2;
#endif
    return SID_LINK_TYPE_3;
  }
}

static bool link_switch(app_context_t *app_context, struct sid_config *config)
{
  enum sid_link_type current_link = config->link_mask;
  enum sid_link_type next_link = get_next_link(config->link_mask);

  if (current_link != next_link) {
    if (init_and_start_link(app_context, config, next_link) != 0) {
      return false;
    }
  } else {
    SL_SID_LOG_APP_WARNING("only one link available on this platform");
  }

  return true;
}

static void send_counter_update(app_context_t *app_context)
{
  char counter_buff[10] = { 0 };

  if (app_context->state == STATE_SIDEWALK_READY
      || app_context->state == STATE_SIDEWALK_SECURE_CONNECTION) {
    SL_SID_LOG_APP_INFO("sending counter update, counter: %d", app_context->counter);

    // buffer for str representation of integer value
    snprintf(counter_buff, sizeof(counter_buff), "%d", app_context->counter);

    struct sid_msg msg = {
      .data = (void *)counter_buff,
      .size = sizeof(counter_buff)
    };
    struct sid_msg_desc desc = {
      .type = SID_MSG_TYPE_NOTIFY,
      .link_type = SID_LINK_TYPE_ANY,
    };

    sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, &msg, &desc);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("send message failed, error: %d", (int)ret);
    } else {
      SL_SID_LOG_APP_INFO("message queued");
      SL_SID_LOG_APP_INFO("link type: %x, msg id: %u, msg size: %u, msg type: %d, ack requested: %d, ttl: %d, max retry: %d, additional attr: %d",
                          desc.link_type,
                          desc.id,
                          msg.size,
                          (int)desc.type,
                          desc.msg_desc_attr.tx_attr.request_ack,
                          desc.msg_desc_attr.tx_attr.ttl_in_seconds,
                          desc.msg_desc_attr.tx_attr.num_retries,
                          desc.msg_desc_attr.tx_attr.additional_attr);
      SL_SID_LOG_APP_HEXDUMP_INFO((const void *)msg.data, msg.size);
    }

    app_context->counter++;
  } else {
    SL_SID_LOG_APP_ERROR("sidewalk not ready yet");
  }
}

static void factory_reset(app_context_t *context)
{
  sid_error_t ret = sid_set_factory_reset(context->sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("factory reset failed, error: %d", (int)ret);

    NVIC_SystemReset();
  } else {
    SL_SID_LOG_APP_INFO("factory reset request accepted");
  }
}

static void get_time(app_context_t *context)
{
  struct sid_timespec curr_time;
  sid_error_t ret = sid_get_time(context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
  if (ret == SID_ERROR_NONE) {
    SL_SID_LOG_APP_INFO("current time: %.02d.%.02d", (int)curr_time.tv_sec, (int)curr_time.tv_nsec);
  } else {
    SL_SID_LOG_APP_ERROR("get time failed, error: %d", (int)ret);
  }
}

static void get_mtu(app_context_t *context)
{
  size_t mtu;
  sid_error_t ret = sid_get_mtu(context->sidewalk_handle, application_context.current_link_type, &mtu);
  if (ret == SID_ERROR_NONE) {
    SL_SID_LOG_APP_INFO("current MTU: %d", mtu);
  } else {
    SL_SID_LOG_APP_ERROR("get MTU failed, error: %d", (int)ret);
  }
}
