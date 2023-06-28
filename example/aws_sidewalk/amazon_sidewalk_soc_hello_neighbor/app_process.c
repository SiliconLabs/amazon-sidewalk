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
#include <string.h>

#include "app_process.h"
#include "app_init.h"
#include "app_assert.h"
#include "app_log.h"
#include "sid_api.h"
#include "sl_sidewalk_common_config.h"
#include "sl_malloc.h"

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
 * @param[in] isr If the Queue is used from ISR then it is handled inside.
 ******************************************************************************/
static void queue_event(QueueHandle_t queue, enum event_type event, bool in_isr);

/*******************************************************************************
 * Function to send updated counter
 *
 * @param[in] app_context The context which is applicable for the current application
 ******************************************************************************/
static void send_counter_update(app_context_t *app_context);

/*******************************************************************************
 * Function to send an array of 0x31 (ASCII symbol '1') with the length specified in `len`
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] config The configuration parameters
 * @param[in] len Data length
 ******************************************************************************/
static void send(app_context_t *app_context, struct sid_config *config, uint8_t len);

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
 * Function to switch between FSK/CSS modulation
 *
 * @param[out] app_context The context which is applicable for the current application
 * @param[out] config The configuration parameters
 *
 * @returns #true           on success
 * @returns #false          on failure
 ******************************************************************************/
static bool toggle_fsk_css_switch(app_context_t *app_context, struct sid_config *config);

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
// uint8_t type arguments
static uint8_t cli_arg_uint8_t;

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
        app_log_error("app: failed to deinitialize sidewalk, link_mask:%x, err:%d", (int)link_mask, (int)ret);
        goto error;
      }
    }

    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;
    // Initialise sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
      app_log_error("app: failed to initialize sidewalk link_mask:%x, err:%d", (int)link_mask, (int)ret);
      goto error;
    }

    // Register sidewalk handler to the application context
    context->sidewalk_handle = sid_handle;
    // Start the sidewalk stack
    ret = sid_start(sid_handle, link_mask);
    if (ret != SID_ERROR_NONE) {
      app_log_error("app: failed to start sidewalk, link_mask:%x, err:%d", (int)link_mask, (int)ret);
      goto error;
    }
  }
  application_context.current_link_type = link_mask;

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

void main_thread(void * context)
{
  //Creating application context
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
    .on_event          = on_sidewalk_event,              /* Called from ISR context */
    .on_msg_received   = on_sidewalk_msg_received,       /* Called from sid_process() */
    .on_msg_sent       = on_sidewalk_msg_sent,           /* Called from sid_process() */
    .on_send_error     = on_sidewalk_send_error,         /* Called from sid_process() */
    .on_status_changed = on_sidewalk_status_changed,     /* Called from sid_process() */
    .on_factory_reset  = on_sidewalk_factory_reset,      /* Called from sid_process() */
  };

  // Set configuration parameters
  struct sid_config config =
  {
    .link_mask = 0,
    .time_sync_periodicity_seconds = 7200,
    .callbacks   = &event_callbacks,
    .link_config = NULL,
    .sub_ghz_link_config = NULL,
  };

  // Queue creation for the sidewalk events
  g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(g_event_queue != NULL, "queue creation failed");

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  config.sub_ghz_link_config = app_get_sub_ghz_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  config.link_config = app_get_ble_config();
#endif

  if (init_and_start_link(&application_context, &config, link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK)) != 0) {
    goto error;
  }

#if defined(SL_BLE_SUPPORTED)
  application_context.connection_request = false;
#endif

  // Initialize to not ready state
  application_context.state = STATE_SIDEWALK_NOT_READY;

  // Assign queue to the application context
  application_context.event_queue = g_event_queue;

  while (1) {
    enum event_type event = EVENT_TYPE_INVALID;

    if (xQueueReceive(application_context.event_queue, &event, portMAX_DELAY) == pdTRUE) {
      // State machine for Sidewalk events
      switch (event) {
        case EVENT_TYPE_SIDEWALK:
          sid_process(application_context.sidewalk_handle);
          break;

        case EVENT_TYPE_SEND_COUNTER_UPDATE:
          app_log_info("app: send counter update event");

          if (application_context.state == STATE_SIDEWALK_READY) {
            send_counter_update(&application_context);
          } else {
            app_log_warning("app: sidewalk not ready");
          }
          break;

        case EVENT_TYPE_SEND:
          app_log_info("app: send event");

          if (application_context.state == STATE_SIDEWALK_READY) {
            send(&application_context, &config, cli_arg_uint8_t);
          } else {
            app_log_warning("app: sidewalk not ready");
          }
          break;

        case EVENT_TYPE_GET_TIME:
          app_log_info("app: get time event");

          get_time(&application_context);
          break;

        case EVENT_TYPE_GET_MTU:
          app_log_info("app: get MTU event");

          get_mtu(&application_context);
          break;

        case EVENT_TYPE_FACTORY_RESET:
          app_log_info("app: factory reset event");

          factory_reset(&application_context);
          break;

        case EVENT_TYPE_FSK_CSS_SWITCH:
          app_log_info("app: FSK/CSS switch event");

          if (toggle_fsk_css_switch(&application_context, &config) != true) {
            goto error;
          }
          break;

        case EVENT_TYPE_REGISTERED:
          if (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE != SL_SIDEWALK_COMMON_REGISTRATION_LINK) {
            if (init_and_start_link(&application_context, &config, link_type_to_link_mask(SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE)) != 0) {
              goto error;
            }
          }
          break;

#if defined(SL_BLE_SUPPORTED)
        case EVENT_TYPE_CONNECTION_REQUEST:
          app_log_info("app: connection request event");

          toggle_connection_request(&application_context);
          break;
#endif

        default:
          app_log_error("app: unexpected event: %d", (int)event);
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
  app_log_error("app: fatal error");

  vTaskDelete(NULL);
}

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
/*******************************************************************************
 * Button handler callback
 * @param[in] handle Button handler
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      if (application_context.current_link_type & SID_LINK_TYPE_1) {
#if defined(SL_BLE_SUPPORTED)
        app_trigger_connection_request();
#endif
      } else {
        app_trigger_fsk_css_switch();
      }
    } else if (&sl_button_btn1 == handle) {
      app_trigger_send_counter_update();
    }
  }
}
#endif

#if defined(SL_BLE_SUPPORTED)
static void toggle_connection_request(app_context_t *context)
{
  if (context->state == STATE_SIDEWALK_READY) {
    app_log_info("app: sidewalk ready, operation not valid");
  } else {
    bool next = !context->connection_request;

    app_log_info("app: %s connection request", next ? "set" : "clear");

    sid_error_t ret = sid_ble_bcn_connection_request(context->sidewalk_handle, next);
    if (ret == SID_ERROR_NONE) {
      context->connection_request = next;
    } else {
      app_log_error("app: connection request failed: %d", (int)ret);
    }
  }
}
#endif

void app_trigger_switching_to_default_link(void)
{
  queue_event(g_event_queue, EVENT_TYPE_REGISTERED, true);

  app_log_info("app: initialising default link");
}

void app_trigger_fsk_css_switch(void)
{
  queue_event(g_event_queue, EVENT_TYPE_FSK_CSS_SWITCH, true);

  app_log_info("app: FSK/CSS switch user event");
}

void app_trigger_send_counter_update(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SEND_COUNTER_UPDATE, true);

  app_log_info("app: send counter update user event");
}

void app_trigger_factory_reset(void)
{
  queue_event(g_event_queue, EVENT_TYPE_FACTORY_RESET, true);

  app_log_info("app: factory reset user event");
}

void app_trigger_send(uint8_t len)
{
  cli_arg_uint8_t = len;
  queue_event(g_event_queue, EVENT_TYPE_SEND, true);

  app_log_info("app: send user event");
}

void app_trigger_get_time(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_TIME, true);

  app_log_info("app: get time user event");
}

void app_trigger_get_mtu(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_MTU, true);

  app_log_info("app: get mtu user event");
}

#if defined(SL_BLE_SUPPORTED)
void app_trigger_connection_request(void)
{
  queue_event(g_event_queue, EVENT_TYPE_CONNECTION_REQUEST, true);

  app_log_info("app: connection request user event");
}
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static void queue_event(QueueHandle_t queue,
                        enum event_type event,
                        bool in_isr)
{
  // Check if queue_event was called from ISR
  if (in_isr) {
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
  app_context_t *app_context = (app_context_t *)context;
  // Issue sidewalk event to the queue
  queue_event(app_context->event_queue, EVENT_TYPE_SIDEWALK, in_isr);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context)
{
  UNUSED(context);
  app_log_info("app: received message (type: %d, id: %u, size: %u)", (int)msg_desc->type, msg_desc->id, msg->size);
  app_log_info("app: %s", (char *) msg->data);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  UNUSED(context);
  app_log_info("app: sent message (type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  UNUSED(context);
  app_log_error("app: failed to send message (type: %d, id: %u, err: %d)",
                (int)msg_desc->type, msg_desc->id, (int)error);
}

/*******************************************************************************
 * Sidewalk Status change handler
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  app_context_t *app_context = (app_context_t *)context;

  app_log_info("app: sidewalk status changed: %d", (int)status->state);

  switch (status->state) {
    case SID_STATE_READY:
      app_context->state = STATE_SIDEWALK_READY;
      break;

    case SID_STATE_NOT_READY:
      app_context->state = STATE_SIDEWALK_NOT_READY;
      break;

    case SID_STATE_ERROR:
      app_log_error("app: sidewalk state error: %d", (int)sid_get_error(app_context->sidewalk_handle));
      break;

    case SID_STATE_SECURE_CHANNEL_READY:
      app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
      break;
  }

  if (status->detail.registration_status == SID_STATUS_REGISTERED) {
    app_trigger_switching_to_default_link();
  }

  app_log_info("app: registration status: %d, time sync status: %d, link status: %d",
               (int) status->detail.registration_status, (int) status->detail.time_sync_status,
               (int) status->detail.link_status_mask);
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
  app_log_info("app: factory reset notification received from sid api");
  // This is the callback function of the factory reset and as the last step a reset is applied.
  NVIC_SystemReset();
}

static bool toggle_fsk_css_switch(app_context_t *app_context, struct sid_config *config)
{
#if !defined(SL_CSS_SUPPORTED)
  (void)app_context;
  (void)config;
  app_log_warning("app: CSS is not supported for this target");
  return true;
#else
  enum sid_link_type current_link_type = config->link_mask;

  if (current_link_type == SID_LINK_TYPE_2) {
    if (init_and_start_link(app_context, config, SID_LINK_TYPE_3) != 0) {
      return false;
    }
    app_log_info("app: Switching to CSS...");
  } else if (current_link_type == SID_LINK_TYPE_3) {
    if (init_and_start_link(app_context, config, SID_LINK_TYPE_2) != 0) {
      return false;
    }
    app_log_info("app: Switching to FSK...");
  } else {
    app_log_warning("app: FSK/CSS switch can not be performed");
  }

  return true;
#endif
}

static void send_counter_update(app_context_t *app_context)
{
  if (app_context->state == STATE_SIDEWALK_READY
      || app_context->state == STATE_SIDEWALK_SECURE_CONNECTION) {
    app_log_info("app: sending counter update: %d", app_context->counter);

    struct sid_msg msg = {
      .data = (uint8_t *)&app_context->counter,
      .size = sizeof(uint8_t)
    };
    struct sid_msg_desc desc = {
      .type = SID_MSG_TYPE_NOTIFY,
      .link_type = SID_LINK_TYPE_ANY,
    };

    sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, &msg, &desc);
    if (ret != SID_ERROR_NONE) {
      app_log_error("app: failed queueing data: %d", (int)ret);
    } else {
      app_log_info("app: queued data message id: %u", desc.id);
    }

    app_context->counter++;
  } else {
    app_log_error("app: sidewalk is not ready yet");
  }
}

static void send(app_context_t *app_context, struct sid_config *config, uint8_t len)
{
  size_t mtu = 0;
  sid_error_t ret = sid_get_mtu(app_context->sidewalk_handle, config->link_mask, &mtu);
  if (ret == SID_ERROR_NONE) {
    if (len > 0 && len <= mtu) {
      if (app_context->state == STATE_SIDEWALK_READY || app_context->state == STATE_SIDEWALK_SECURE_CONNECTION) {
        app_log_info("app: sending %d bytes", (int)len);

        uint8_t *data = (uint8_t *)sl_malloc(len * sizeof(uint8_t));
        if (data != NULL) {
          memset(data, 0x31, len);

          struct sid_msg msg = {
            .data = data,
            .size = len
          };
          struct sid_msg_desc desc = {
            .type = SID_MSG_TYPE_NOTIFY,
            .link_type = SID_LINK_TYPE_ANY,
          };

          sid_error_t ret = sid_put_msg(app_context->sidewalk_handle, &msg, &desc);
          if (ret != SID_ERROR_NONE) {
            app_log_error("app: failed queueing data: %d", (int)ret);
          } else {
            app_log_info("app: queued data message id: %u", desc.id);
          }

          sl_free(data);
          data = NULL;
        } else {
          app_log_error("app: memory allocation failed for the message");
        }
      } else {
        app_log_error("app: sidewalk is not ready yet");
      }
    } else {
      app_log_error("app: Unsupported argument");
    }
  } else {
    app_log_error("app: failed getting MTU: %d", ret);
  }
}

static void factory_reset(app_context_t *context)
{
  sid_error_t ret = sid_set_factory_reset(context->sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: notification of factory reset to sid api failed");

    NVIC_SystemReset();
  } else {
    app_log_info("app: waiting for sid api to notify to proceed with factory reset");
  }
}

static void get_time(app_context_t *context)
{
  struct sid_timespec curr_time;
  sid_error_t ret = sid_get_time(context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
  if (ret == SID_ERROR_NONE) {
    app_log_info("app: current time: %d.%d", (int) curr_time.tv_sec, (int) curr_time.tv_nsec);
  } else {
    app_log_error("app: failed getting time: %d", ret);
  }
}

static void get_mtu(app_context_t *context)
{
  size_t mtu;
  sid_error_t ret = sid_get_mtu(context->sidewalk_handle, SID_LINK_TYPE_2, &mtu);
  if (ret == SID_ERROR_NONE) {
    app_log_info("app: current mtu: %d", mtu);
  } else {
    app_log_error("app: failed getting MTU: %d", ret);
  }
}
