/***************************************************************************//**
 * @file
 * @brief app_process.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlibgit
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
#include "app_ble_config.h"
#include "app_subghz_config.h"
#include "app_init.h"
#include "app_assert.h"
#include "app_log.h"
#if defined(EFR32MG21) || defined(EFR32MG24)
#include "sl_simple_button_instances.h"
#endif
#include "sl_bt_api.h"

#include "sid_api.h"

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

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// Queue for sending events
static QueueHandle_t g_event_queue;

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
        app_log_error("App - failed to deinitialize sidewalk, link_mask:%x, err:%d", (int)link_mask, (int)ret);
        goto error;
      }
    }

    struct sid_handle *sid_handle = NULL;
    config->link_mask = link_mask;
    // Initialise sidewalk
    ret = sid_init(config, &sid_handle);
    if (ret != SID_ERROR_NONE) {
      app_log_error("App - failed to initialize sidewalk link_mask:%x, err:%d", (int)link_mask, (int)ret);
      goto error;
    }

    // Register sidewalk handler to the application context
    context->sidewalk_handle = sid_handle;

    // Start the sidewalk stack
    ret = sid_start(sid_handle, link_mask);
    if (ret != SID_ERROR_NONE) {
      app_log_error("App - failed to start sidewalk, link_mask:%x, err:%d", (int)link_mask, (int)ret);
      goto error;
    }
  }
  return 0;

  error:
  context->sidewalk_handle = NULL;
  config->link_mask = 0;
  return -1;
}

void main_thread(void * context)
{
  // Creating application context
  app_context_t *app_context = (app_context_t *)context;

  // Register the callback functions and the context
  struct sid_event_callbacks event_callbacks =
  {
    .context           = app_context,
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
    .link_config = app_get_ble_config(),
    .sub_ghz_link_config = app_get_sub_ghz_config(),
  };

  struct sid_handle *sid_handle = NULL;

  if (init_and_start_link(app_context, &config, SID_LINK_TYPE_1) != 0) {
    goto error;
  }
  sid_handle = app_context->sidewalk_handle;

  // Queue creation for the sidewalk events
  g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(g_event_queue != NULL, "queue creation failed");

  // Initialize to not ready state
  app_context->state = STATE_SIDEWALK_NOT_READY;

  // Assign queue to the application context
  app_context->event_queue = g_event_queue;

  /*
     THIS IS FOR MAXIMIZING TX POWER FOR ADVERTISING.
   */
  sl_bt_advertiser_stop(0);

  int16_t support_min;
  int16_t support_max;
  int16_t set_min;
  int16_t set_max;
  int16_t rf_path_gain;
  int16_t adv_set;
  sl_bt_system_get_tx_power_setting(&support_min, &support_max, &set_min, &set_max, &rf_path_gain);
  sl_bt_advertiser_set_tx_power(0, set_max, &adv_set);
  sl_bt_advertiser_start(0, sl_bt_advertiser_user_data, sl_bt_advertiser_connectable_scannable);
  /*
     END OF TX POWER
   */

  while (1) {
    enum event_type event = EVENT_TYPE_INVALID;

    if (xQueueReceive(app_context->event_queue, &event, portMAX_DELAY) == pdTRUE) {
      // State machine for Sidewalk events
      switch (event) {
        case EVENT_TYPE_SIDEWALK:
          sid_process(sid_handle);
          break;

        case EVENT_TYPE_SEND_COUNTER_UPDATE:
          if (config.link_mask != SID_LINK_TYPE_2 && config.link_mask != SID_LINK_TYPE_3) {
            if (init_and_start_link(app_context, &config, SID_LINK_TYPE_2) != 0) {
              goto error;
            }
          }
          app_log_info("App - send counter update event");

          if (app_context->state == STATE_SIDEWALK_READY) {
            send_counter_update(app_context);
          } else {
            app_log_warning("App - sidewalk not ready");
          }
          break;

        case EVENT_TYPE_GET_TIME:
          app_log_info("App - get time event");

          get_time(app_context);
          break;

        case EVENT_TYPE_GET_MTU:
          app_log_info("App - get MTU event");

          get_mtu(app_context);
          break;

        case EVENT_TYPE_FACTORY_RESET:
          app_log_info("App - factory reset event");

          factory_reset(app_context);
          break;

        case EVENT_TYPE_FSK_CSS_SWITCH:
          app_log_info("App - FSK/CSS switch event");

          if (toggle_fsk_css_switch(app_context, &config) != true) {
            goto error;
          }
          break;

        default:
          app_log_error("App - unexpected event: %d", (int)event);
          break;
      }
    }
  }

  error:
// If error happens deinit sidewalk
  if (sid_handle != NULL) {
    sid_stop(sid_handle, config.link_mask);
    sid_deinit(sid_handle);
    app_context->sidewalk_handle = NULL;
  }
  app_log_error("App - fatal error");

  vTaskDelete(NULL);
}

#if defined(EFR32MG21) || defined(EFR32MG24)
/*******************************************************************************
 * Button handler callback
 * @param[in] handle Button handler
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      app_trigger_fsk_css_switch();
    } else if (&sl_button_btn1 == handle) {
      app_trigger_send_counter_update();
    }
  }
}
#endif

void app_trigger_fsk_css_switch(void)
{
  queue_event(g_event_queue, EVENT_TYPE_FSK_CSS_SWITCH, true);

  app_log_info("App - FSK/CSS switch user event");
}

void app_trigger_send_counter_update(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SEND_COUNTER_UPDATE, true);

  app_log_info("App - send counter update user event");
}

void app_trigger_factory_reset(void)
{
  queue_event(g_event_queue, EVENT_TYPE_FACTORY_RESET, true);

  app_log_info("App - factory reset user event");
}

void app_trigger_get_time(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_TIME, true);

  app_log_info("App - get time user event");
}

void app_trigger_get_mtu(void)
{
  queue_event(g_event_queue, EVENT_TYPE_GET_MTU, true);

  app_log_info("App - get mtu user event");
}

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
  app_log_info("App - received message (type: %d, id: %u, size %u)", (int)msg_desc->type, msg_desc->id, msg->size);
  app_log_info("App - %s", (char *) msg->data);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  UNUSED(context);
  app_log_info("App - sent message (type: %d, id: %u)", (int)msg_desc->type, msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  UNUSED(context);
  app_log_error("App - failed to send message (type: %d, id: %u, err: %d)",
                (int)msg_desc->type, msg_desc->id, (int)error);
}

/*******************************************************************************
 * Sidewalk Status change handler
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  app_context_t *app_context = (app_context_t *)context;

  app_log_info("App - sidewalk status changed: %d", (int)status->state);

  switch (status->state) {
    case SID_STATE_READY:
      app_context->state = STATE_SIDEWALK_READY;
      break;

    case SID_STATE_NOT_READY:
      app_context->state = STATE_SIDEWALK_NOT_READY;
      break;

    case SID_STATE_ERROR:
      app_log_error("App - sidewalk state error: %d", (int)sid_get_error(app_context->sidewalk_handle));
      break;

    case SID_STATE_SECURE_CHANNEL_READY:
      app_context->state = STATE_SIDEWALK_SECURE_CONNECTION;
      break;
  }

  app_log_info("App - registration status: %d, time sync status: %d, link status: %d",
               (int) status->detail.registration_status, (int) status->detail.time_sync_status,
               (int) status->detail.link_status_mask);
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
  app_log_info("App - factory reset notification received from sid api");
  // This is the callback function of the factory reset and as the last step a reset is applied.
  NVIC_SystemReset();
}

static bool toggle_fsk_css_switch(app_context_t *app_context, struct sid_config *config)
{
  enum sid_link_type current_link_type = config->link_mask;

  if (current_link_type == SID_LINK_TYPE_2) {
    if (init_and_start_link(app_context, config, SID_LINK_TYPE_3) != 0) {
      return false;
    }
    app_log_info("App - Switching to CSS...");
  } else if (current_link_type == SID_LINK_TYPE_3) {
    if (init_and_start_link(app_context, config, SID_LINK_TYPE_2) != 0) {
      return false;
    }
    app_log_info("App - Switching to FSK...");
  } else {
    app_log_warning("App - FSK/CSS switch can not be performed");
  }

  return true;
}

static void send_counter_update(app_context_t *app_context)
{
  if (app_context->state == STATE_SIDEWALK_READY
      || app_context->state == STATE_SIDEWALK_SECURE_CONNECTION) {
    app_log_info("App - sending counter update: %d", app_context->counter);

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
      app_log_error("App - failed queueing data: %d", (int)ret);
    } else {
      app_log_info("App - queued data message id: %u", desc.id);
    }

    app_context->counter++;
  } else {
    app_log_error("App - sidewalk is not ready yet");
  }
}

static void factory_reset(app_context_t *context)
{
  sid_error_t ret = sid_set_factory_reset(context->sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - notification of factory reset to sid api failed");

    NVIC_SystemReset();
  } else {
    app_log_info("App - waiting for sid api to notify to proceed with factory reset");
  }
}

static void get_time(app_context_t *context)
{
  struct sid_timespec curr_time;
  sid_error_t ret = sid_get_time(context->sidewalk_handle, SID_GET_GPS_TIME, &curr_time);
  if (ret == SID_ERROR_NONE) {
    app_log_info("App - current time: %d.%d", (int) curr_time.tv_sec, (int) curr_time.tv_nsec);
  } else {
    app_log_error("App - failed getting time: %d", ret);
  }
}

static void get_mtu(app_context_t *context)
{
  size_t mtu;
  sid_error_t ret = sid_get_mtu(context->sidewalk_handle, SID_LINK_TYPE_2, &mtu);
  if (ret == SID_ERROR_NONE) {
    app_log_info("App - current mtu: %d", mtu);
  } else {
    app_log_error("App - failed getting MTU: %d", ret);
  }
}
