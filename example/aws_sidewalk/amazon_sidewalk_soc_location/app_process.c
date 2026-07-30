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

#include "FreeRTOS.h"
#include "queue.h"

#include "sl_component_catalog.h"
#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "app_button_press.h"
#endif
#include "app_process.h"
#include "app_assert.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_utils.h"
#include "sid_api.h"
#include "sl_sidewalk_common_config.h"

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

#if defined(SL_BLE_SUPPORTED)
#include "app_ble_config.h"
#include "sl_bt_api.h"
#endif

#if defined(SL_CATALOG_SIDEWALK_LOCATION_CLI_PRESENT)
#include "sl_sidewalk_location_cli.h"
#endif

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "sl_simple_button_instances.h"
#endif
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Sidewalk Events
enum event_type{
  EVENT_TYPE_SIDEWALK = 0,
#if defined(SL_BLE_SUPPORTED)
  EVENT_TYPE_CONNECTION_REQUEST,
#endif
  EVENT_TYPE_SEND_COUNTER_UPDATE,
  EVENT_TYPE_FACTORY_RESET,
  EVENT_TYPE_LINK_SWITCH,
  EVENT_TYPE_REGISTERED,
  EVENT_TYPE_SEND,
  EVENT_TYPE_INVALID,
  EVENT_TYPE_LOCATION_INIT,
  EVENT_TYPE_LOCATION_DEINIT,
  EVENT_TYPE_LOCATION_SCAN_AND_SEND_L1,
  EVENT_TYPE_LOCATION_SCAN_AND_SEND_L3,
  EVENT_TYPE_LOCATION_SCAN_AND_SEND_L4,
#if defined(SL_LOCATION_FULL)
  EVENT_TYPE_LOCATION_SCAN_ONLY_L3,
  EVENT_TYPE_LOCATION_SCAN_ONLY_L4,
  EVENT_TYPE_LOCATION_SEND_ONLY_L3,
  EVENT_TYPE_LOCATION_SEND_ONLY_L4,
  EVENT_TYPE_LOCATION_ALM_START
#endif // defined(SL_LOCATION_FULL)
};

// Sidewalk States defined in application context
enum app_state{
  STATE_INIT = 0,
  STATE_SIDEWALK_READY,
  STATE_SIDEWALK_NOT_READY,
  STATE_SIDEWALK_SECURE_CONNECTION
};

// Application context
typedef struct app_context{
  struct sid_handle *sidewalk_handle;
  enum app_state state;
  uint8_t counter;
  uint32_t current_link_type;
#if defined(SL_BLE_SUPPORTED)
  bool connection_request;
#endif
} app_context_t;

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

/*******************************************************************************
 * Function to initialize the location
 ******************************************************************************/
static void location_init(void);

/*******************************************************************************
 * Function to deinitialize the location
 ******************************************************************************/
static void location_deinit(void);

/*******************************************************************************
 * Function to send the location
 *
 * @param[in] effort Requested effort level
 ******************************************************************************/
static void location_scan_and_send(enum sid_location_effort_mode effort);

#if defined(SL_LOCATION_FULL)
/*******************************************************************************
 * Function to scan the location
 *
 * @param[in] effort Requested effort level
 ******************************************************************************/
static void location_scan_only(enum sid_location_effort_mode effort);

/*******************************************************************************
 * Function to send the location buffer
 *
 * @param[in] effort Requested effort level
 ******************************************************************************/
static void location_send_only(enum sid_location_effort_mode effort);

/*******************************************************************************
 * Function to start the location alarm
 ******************************************************************************/
static void location_alm_start(void);
#endif // defined(SL_LOCATION_FULL)

/*******************************************************************************
 * Function to get the next link
 *
 * @param[in] current_link Current link
 *
 * @returns Next link
 ******************************************************************************/
static enum sid_link_type get_next_link(enum sid_link_type current_link);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

extern struct sid_location_config sl_sidewalk_location_cli_config;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// Queue for sending events
static QueueHandle_t app_event_queue = NULL;

static app_context_t application_context;

#if defined(SL_LOCATION_FULL)
static sl_sidewalk_location_cli_buffer_t location_buffer ={.buffer = {0}, .size = 0};
#endif // defined(SL_LOCATION_FULL)
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

  return 0;

  error:
  context->sidewalk_handle = NULL;
  config->link_mask = 0;
  return -1;
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

void main_thread(void *context)
{
  // Creating application context
  (void)context;

  // Application context creation
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

  // Queue creation for the sidewalk events
  app_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  app_assert(app_event_queue != NULL, "queue creation failed");

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

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  config.sub_ghz_link_config = app_get_sub_ghz_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  config.link_config = app_get_ble_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  application_context.connection_request = false;
#endif

  // Initialize to not ready state
  application_context.state = STATE_SIDEWALK_NOT_READY;

  if (init_and_start_link(&application_context, &config, link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK)) != 0) {
    goto error;
  }

  SL_SID_LOG_APP_INFO("main task started");

  while (1) {
    enum event_type event = EVENT_TYPE_INVALID;

    if (xQueueReceive(app_event_queue, &event, portMAX_DELAY) == pdTRUE) {
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

          if (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE != SL_SIDEWALK_COMMON_REGISTRATION_LINK
              && init_and_start_link(&application_context,
                                     &config,
                                     link_type_to_link_mask(SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE)) != 0) {
            goto error;
          }
          break;

#if defined(SL_BLE_SUPPORTED)
        case EVENT_TYPE_CONNECTION_REQUEST:
          SL_SID_LOG_APP_INFO("BLE connection request event");

          toggle_connection_request(&application_context);
          break;
#endif

        case EVENT_TYPE_LOCATION_INIT:
          SL_SID_LOG_APP_INFO("location init event");
          location_init();
          break;

        case EVENT_TYPE_LOCATION_DEINIT:
          SL_SID_LOG_APP_INFO("location deinit event");
          location_deinit();
          break;

        case EVENT_TYPE_LOCATION_SCAN_AND_SEND_L1:
          SL_SID_LOG_APP_INFO("location scan_and_send L1 event");
          location_scan_and_send(SID_LOCATION_EFFORT_L1);
          break;

        case EVENT_TYPE_LOCATION_SCAN_AND_SEND_L3:
          SL_SID_LOG_APP_INFO("location scan_and_send L3 event");
          location_scan_and_send(SID_LOCATION_EFFORT_L3);
          break;

        case EVENT_TYPE_LOCATION_SCAN_AND_SEND_L4:
          SL_SID_LOG_APP_INFO("location scan_and_send L4 event");
          location_scan_and_send(SID_LOCATION_EFFORT_L4);
          break;

#if defined(SL_LOCATION_FULL)
        case EVENT_TYPE_LOCATION_SCAN_ONLY_L3:
          SL_SID_LOG_APP_INFO("location scan_only L3 event");
          location_scan_only(SID_LOCATION_EFFORT_L3);
          break;

        case EVENT_TYPE_LOCATION_SCAN_ONLY_L4:
          SL_SID_LOG_APP_INFO("location scan_only L4 event");
          location_scan_only(SID_LOCATION_EFFORT_L4);
          break;

        case EVENT_TYPE_LOCATION_SEND_ONLY_L3:
          SL_SID_LOG_APP_INFO("location send_only L3 event");
          location_send_only(SID_LOCATION_EFFORT_L3);
          break;

        case EVENT_TYPE_LOCATION_SEND_ONLY_L4:
          SL_SID_LOG_APP_INFO("location send_only L4 event");
          location_send_only(SID_LOCATION_EFFORT_L4);
          break;

          case EVENT_TYPE_LOCATION_ALM_START:
          SL_SID_LOG_APP_INFO("location alm start event");
          location_alm_start();
          break;
#endif // defined(SL_LOCATION_FULL)

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
      app_trigger_send_counter_update();
    }
#else // All others target others than KG100S
    (void)duration;
    app_trigger_link_switch();
#endif  // !defined(SL_CATALOG_BTN1_PRESENT)
  } else { // PB1
#if !defined(SL_CATALOG_BTN1_PRESENT) //KG100S
    SL_SID_LOG_APP_ERROR("kg100s BTN1 not configured");
#else
    app_trigger_send_counter_update();
#endif // !defined(SL_CATALOG_BTN1_PRESENT)
  }
}
#endif

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

static void location_init(void)
{
  sid_error_t res = sid_location_init(application_context.sidewalk_handle,&sl_sidewalk_location_cli_config);
  SL_SID_LOG_APP_INFO("location init result: %d", res);
}

static void location_deinit(void)
{
  sid_error_t res = sid_location_deinit(application_context.sidewalk_handle);
  SL_SID_LOG_APP_INFO("location deinit result: %d", res);
}

static void location_scan_and_send(enum sid_location_effort_mode effort)
{
  struct sid_location_run_config config = {.type = SID_LOCATION_SCAN_AND_SEND, .mode = effort};

  sid_error_t res = sid_location_run(application_context.sidewalk_handle, &config, 0);
  SL_SID_LOG_APP_INFO("location scan_and_send result: %d", res);
}

#if defined(SL_LOCATION_FULL)
static void location_scan_only(enum sid_location_effort_mode effort)
{
  struct sid_location_run_config config = {
    .type = SID_LOCATION_SCAN_ONLY,
    .mode = effort,
  };

  sid_error_t res = sid_location_run(application_context.sidewalk_handle, &config, 0);
  SL_SID_LOG_APP_INFO("location scan_only result: %d", res);
}

static void location_send_only(enum sid_location_effort_mode effort)
{
  struct sid_location_run_config config = {
    .type = SID_LOCATION_SEND_ONLY,
    .mode = effort,
    .buffer = location_buffer.buffer,
    .size = location_buffer.size,
  };

  sid_error_t res = sid_location_run(application_context.sidewalk_handle, &config, 0);
  SL_SID_LOG_APP_INFO("location send_only result: %d", res);
}

void sl_sidewalk_location_cli_handle_result(const uint8_t* const payload, size_t size)
{
  SL_SID_LOG_APP_INFO("storing location data in the app, or successful send buffer");
  if (size <= LOCATION_BUFFER_SIZE) {
    location_buffer.size = size;
    memcpy(location_buffer.buffer, payload, size);
  } else {
    SL_SID_LOG_APP_ERROR("location data size is too large");
  }
}

static void location_alm_start(void)
{
  #if SID_SDK_CONFIG_ENABLE_GNSS
  sid_pal_gnss_alm_demod_start();
  SL_SID_LOG_APP_INFO("location alm start");
  #else
  SL_SID_LOG_APP_ERROR("location alm start not supported");
  #endif
}
#endif // defined(SL_LOCATION_FULL)

void app_trigger_switching_to_default_link(void)
{
  queue_event(app_event_queue, EVENT_TYPE_REGISTERED);
}

void app_trigger_link_switch(void)
{
  queue_event(app_event_queue, EVENT_TYPE_LINK_SWITCH);
}

void app_trigger_send_counter_update(void)
{
  if (application_context.current_link_type & SID_LINK_TYPE_1) {
    if (application_context.state == STATE_SIDEWALK_READY) {
      queue_event(app_event_queue, EVENT_TYPE_SEND_COUNTER_UPDATE);
    } else {
      SL_SID_LOG_APP_WARNING("BLE is not connected");
    }
  } else {
    queue_event(app_event_queue, EVENT_TYPE_SEND_COUNTER_UPDATE);
  }
}

void app_trigger_factory_reset(void)
{
  queue_event(app_event_queue, EVENT_TYPE_FACTORY_RESET);
}

#if defined(SL_BLE_SUPPORTED)
void app_trigger_connection_request(void)
{
  if (application_context.current_link_type & SID_LINK_TYPE_1) {
    queue_event(app_event_queue, EVENT_TYPE_CONNECTION_REQUEST);
  } else {
    SL_SID_LOG_APP_WARNING("The current link is not BLE");
  }
}
#endif

void app_trigger_location_init(void)
{
  queue_event(app_event_queue, EVENT_TYPE_LOCATION_INIT);
}

void app_trigger_location_deinit(void)
{
  queue_event(app_event_queue, EVENT_TYPE_LOCATION_DEINIT);
}

void app_trigger_location_scan_and_send(enum sid_location_effort_mode effort)
{
  switch (effort) {
    case SID_LOCATION_EFFORT_L1:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SCAN_AND_SEND_L1);
      break;
#if defined(SL_LOCATION_FULL)
    case SID_LOCATION_EFFORT_L3:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SCAN_AND_SEND_L3);
      break;
    case SID_LOCATION_EFFORT_L4:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SCAN_AND_SEND_L4);
      break;
#endif // defined(SL_LOCATION_FULL)
    default:
      SL_SID_LOG_APP_ERROR("location scan_and_send effort: %d is invalid or not supported", effort);
      break;
  }
}

#if defined(SL_LOCATION_FULL)
void app_trigger_location_scan_only(enum sid_location_effort_mode effort)
{
  switch (effort) {
    case SID_LOCATION_EFFORT_L3:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SCAN_ONLY_L3);
      break;
    case SID_LOCATION_EFFORT_L4:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SCAN_ONLY_L4);
      break;
    default:
      SL_SID_LOG_APP_ERROR("location scan_only invalid argument: %d", effort);
      break;
  }
}

void app_trigger_location_send_only(enum sid_location_effort_mode effort)
{
  switch (effort) {
    case SID_LOCATION_EFFORT_L3:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SEND_ONLY_L3);
      break;
    case SID_LOCATION_EFFORT_L4:
      queue_event(app_event_queue, EVENT_TYPE_LOCATION_SEND_ONLY_L4);
      break;
    default:
      SL_SID_LOG_APP_ERROR("location send_only invalid argument: %d", effort);
      break;
  }
}

void app_trigger_location_alm_start(void)
{
#if SID_SDK_CONFIG_ENABLE_GNSS
  queue_event(app_event_queue, EVENT_TYPE_LOCATION_ALM_START);
#else
  SL_SID_LOG_APP_ERROR("location alm start not supported");
#endif // defined(SID_SDK_CONFIG_ENABLE_GNSS)
}
#endif // defined(SL_LOCATION_FULL)

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
  UNUSED(context);
  // Issue sidewalk event to the queue
  queue_event(app_event_queue, EVENT_TYPE_SIDEWALK);
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
#else
    return SID_LINK_TYPE_1;
#endif
  } else if (current_link == SID_LINK_TYPE_2) {
#if defined(SL_CSS_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to CSS link");
    return SID_LINK_TYPE_3;
#elif defined(SL_BLE_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to BLE link");
    return SID_LINK_TYPE_1;
#else
    return SID_LINK_TYPE_2;
#endif
  } else { // (current_link == SID_LINK_TYPE_3)
#if defined(SL_BLE_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to BLE link");
    return SID_LINK_TYPE_1;
#elif defined(SL_FSK_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to FSK link");
    return SID_LINK_TYPE_2;
#else
    return SID_LINK_TYPE_3;
#endif
  }
}

static bool link_switch(app_context_t *app_context, struct sid_config *config)
{
  enum sid_link_type current_link = config->link_mask;
  enum sid_link_type next_link = get_next_link(config->link_mask);

#if defined(SL_CSS_SUPPORTED)
  struct sid_status status;
  if (next_link == SID_LINK_TYPE_3) {
    sid_get_status(app_context->sidewalk_handle, &status);
    if (status.detail.registration_status != SID_STATUS_REGISTERED) {
      SL_SID_LOG_APP_WARNING("Registration is not allowed on CSS");
      next_link = get_next_link(next_link);
    }
  }
#endif

  if (current_link != next_link) {
    if (init_and_start_link(app_context, config, next_link) != 0) {
      return false;
    }
  }
#if (defined(SL_BLE_SUPPORTED) + defined(SL_FSK_SUPPORTED) + defined(SL_CSS_SUPPORTED)) == 1
  SL_SID_LOG_APP_WARNING("only one link available on this platform");
#endif

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
