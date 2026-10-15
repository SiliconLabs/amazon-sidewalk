/***************************************************************************//**
 * @file
 * @brief app_process.c
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include <string.h>

#include "app_assert.h"
#include "app_link_config.h"
#include "app_process.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "sid_api.h"
#include "sl_component_catalog.h"
#include "sl_sidewalk_common_config.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_web_utils.h"

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
#include "sl_sidewalk_cmd_executor2.h"
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
#include "app_button_press.h"
#include "sl_simple_button_instances.h"
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
//                          External Global Variables
// -----------------------------------------------------------------------------

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
extern const sl_sidewalk_cmd_executor2_cmd_t sl_sidewalk_web_utils_cmd_tbl[];
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#if SL_SIDEWALK_LINK_TO_USE == SL_SIDEWALK_LINK_CSS
#error "Cloud Light does not support CSS as the initial link to use"
#endif // SL_SIDEWALK_LINK_TO_USE == SL_SIDEWALK_LINK_CSS

#define MANAGEMENT_QUEUE_LEN (10U)
#define UPLINK_QUEUE_LEN (15U)
#define DEFAULT_RETRY_PERIOD (20U)

// We simply use the FSK MTU as setup messages will be sent on either BLE or FSK
#define SETUP_MSG_MAX_SIZE (200U)

// Size of buffer for building payload strings for sidewalk messages.
// It is 20 bytes to handle null-terminated strings.
// Before calling sid_put_msg it will be stripped, resulting in 19 byte sid_msg payload fitting all links.
#define RUNTIME_MSG_STR_MAX_SIZE (19U)

// When converting sensor data to stringm, this buffer size is used.
#define SENSOR_DATA_STR_MAX_SIZE (12U)

// Application states
typedef enum {
  APP_STATE_STARTING = 0,
  APP_STATE_WAIT_FOR_URL_AND_READY,
  APP_STATE_WAIT_FOR_URL,
  APP_STATE_WAIT_FOR_READY,
  APP_STATE_CAPABILITIES_IN_PROGRESS,
  APP_STATE_CAPABILITIES_LAST,
  APP_STATE_RUNNING
} app_state_t;

// Application event IDs.
// Note there are management and uplink events, both listed in this enum, but there are separate queues for them.
typedef enum {
  APP_EVENT_ID_M_SIDEWALK_PROCESS = 0,
  APP_EVENT_ID_M_BLE_CONNECTION_REQUEST,
  APP_EVENT_ID_M_START_NORMAL_RUNTIME,
  APP_EVENT_ID_M_LINK_SWITCH,
  APP_EVENT_ID_M_UPDATE_DISPLAY,
  APP_EVENT_ID_U_SEND_HELLO,
  APP_EVENT_ID_U_SEND_CAPABILITIES,
  APP_EVENT_ID_U_SEND_SENSOR_DATA,
  APP_EVENT_ID_INVALID
} app_event_id_t;

typedef struct {
  sl_sidewalk_web_utils_capability_t capability;
  char sensor_value[SENSOR_DATA_STR_MAX_SIZE];
} app_event_data_t;

typedef struct {
  app_event_id_t id;
  app_event_data_t data;
} app_event_t;

typedef struct {
  struct sid_handle *sid_handle;
  enum sid_state sid_state;
  app_state_t app_state;
  QueueHandle_t management_queue;
  QueueHandle_t uplink_queue;
  uint32_t retry_period_in_sec;
  uint16_t waited_msg_id;
#if defined(SL_BLE_SUPPORTED)
  bool ble_connection_request_pending; // to prevent multiple queueing of ble connection request, true from app queuing the request, until the sid status change to ready
#endif // defined(SL_BLE_SUPPORTED)
#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  TimerHandle_t display_update_timer_hnd;
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  TimerHandle_t temperature_measure_timer_hnd;
#endif // defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
} app_rnt_t;
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
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
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context);

/*******************************************************************************
 * Callback method for the case when a sidewalk message is sent
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context);

/*******************************************************************************
 * Callback function if error happened during send operation
 *
 * @param[in] error The error type
 * @param[in] msg_desc Message descriptor
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context);

/*******************************************************************************
 * Callback Function to handle status changes in the Sidewalk context
 *
 * @param[in] status  new status
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context);

/*******************************************************************************
 * Callback function which is called from factory reset sidewalk event
 *
 * @param[in] context The context which is applicable for the current application
 ******************************************************************************/
static void on_sidewalk_factory_reset(void *context);

/*******************************************************************************
 * Event handler to trigger the BLE connection request
 *
 ******************************************************************************/
static inline void handle_ble_connection_request(void);

/*******************************************************************************
 * Event handler to send the initial message
 *
 ******************************************************************************/
static inline void handle_send_hello(void);

/*******************************************************************************
 * Event handler to send the capabilities message
 *
 ******************************************************************************/
static inline void handle_send_capabilities(void);

/*******************************************************************************
 * Event handler to start the normal runtime
 *
 ******************************************************************************/
static inline void handle_start_normal_runtime(void);

/*******************************************************************************
 * Event handler to switch link
 *
 * Expected to be called only in app state APP_STATE_RUNNING, with initialized
 * sid handle, registered state, etc.
 *
 ******************************************************************************/
static inline void handle_link_switch(void);

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
/*******************************************************************************
 * Event handler to update the display
 *
 ******************************************************************************/
static inline void handle_update_display(void);
#endif

/**
 * Event handler to send sensor data message
 *
 * @param[in] app_event  the app event
 ******************************************************************************/
static inline void handle_send_sensor_data(app_event_t* app_event);

/*******************************************************************************
 * Function to convert the configured link value (e.g., SL_SIDEWALK_LINK_BLE) to
 * the corresponding sid_link_type of the sidewalk stack (e.g., SID_LINK_TYPE_1)
 *
 * Note: the resulting sid_link_type value also can be used as link_mask for
 * that single link (i.e., if the mask is not a combination of multiple links)
 *
 * @param[in] link_value  the configured link value to convert
 *
 * @returns sid_link_type  the corresponding link_mask enumeration
 ******************************************************************************/
static enum sid_link_type configured_link_value_to_sid_link_type(uint8_t link_value);

/*******************************************************************************
 * Function to queue a app event to either management or uplink queue to be
 * processed in the main thread.
 *
 * @param[in] queue  the target queue, either management or uplink queue
 * @param[in] app_event  the app event
 * @param[in] to_front  if the event shall be queued to the front of the queue
 ******************************************************************************/
static void queue_app_event(QueueHandle_t queue, app_event_t* app_event, bool to_front);

/*******************************************************************************
 * Function to get the retry period based on the link mask
 *
 * @param[in] link_mask  the link mask
 *
 * @returns retry_period  the retry period in seconds
 ******************************************************************************/
static uint16_t get_retry_period_in_sec(uint32_t link_mask);

/*******************************************************************************
 * Function to get the next link (sid_link_type) based on the current link mask
 *
 * @param[in] current_link_mask  the current link mask, expected to be a single
 * link
 *
 * @returns next_link  the next link
 ******************************************************************************/
static enum sid_link_type get_next_link(uint32_t current_link_mask);

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
/*******************************************************************************
 * Function to start the temperature timer.
 *
 * It changes the period of the timer and starts it. It also triggers the temperature interval report.
 *
 * Note: simply returns if called when the timer is already active.
 *
 ******************************************************************************/
static void start_temperature_timer(void);
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

/*******************************************************************************
 * Function to check if BLE is connected and trigger a connection request if needed
 *
 ******************************************************************************/
static void check_connection(void);

/*******************************************************************************
 * Function to log the received sidewalk message
 *
 * @param[in] sid_payload  the payload of the received sid message, not null
 *            terminated. May be NULL.
 * @param[in] sid_msg_size The size of the received message. May be 0.
 * @param[in] msg_desc  the message descriptor
 ******************************************************************************/
static inline void log_received_sid_msg(const void *sid_payload,
                                        size_t sid_msg_size,
                                        const struct sid_msg_desc *msg_desc);

/*******************************************************************************
 * Function to check if a received sid msg is an URL and if so handle it.
 * Relies on sidewalk web utils component to parse the URL then performs further actions.
 *
 * @param[in] sid_payload  the payload of the received sid message, not null
 *            terminated. May be NULL.
 * @param[in] sid_msg_size The size of the received message. May be 0.
 *
 * @returns true if the message was an URL
 ******************************************************************************/
static bool try_handle_url_msg(const void *sid_payload, size_t sid_msg_size);

/*******************************************************************************
 * Function to process unrecoverable errors.
 * Stops and deinitializes the sidewalk stack, then deletes the main task.
 *
 * This function shall never return.
 *
 ******************************************************************************/
static void process_error(void);

/*******************************************************************************
 * Function to put a message to the sidewalk stack
 *
 * @param[in] msg  the message to put as a null-terminated string
 * @param[in] request_ack  if the message shall be acknowledged
 * @param[in] retries  the number of retries
 *
 * @returns msg_id  the message id
 ******************************************************************************/
static uint16_t put_msg(char *msg, bool request_ack, uint8_t retries);

/*******************************************************************************
 * Function to check if the recently received acknowledgement is actually waited
 * by a previous message
 *
 * @param[in] msg_desc  the message descriptor
 *
 * @returns true if the waited message is acknowledged, false otherwise
 ******************************************************************************/
static bool is_waited_ack(const struct sid_msg_desc *msg_desc);

/*******************************************************************************
 * Function to trigger sidewalk stack processing from main thread
 *
 ******************************************************************************/
static inline void trigger_sid_process(void);

/*******************************************************************************
 * Function to trigger sending a capabilities message
 *
 ******************************************************************************/
static void trigger_send_capabilities(void);

#if defined(SL_CATALOG_BTN0_PRESENT)
/*******************************************************************************
 * Function to trigger the button 0 s counter
 *
 ******************************************************************************/
static inline void trigger_button0_s_counter(void);
#endif

#if defined(SL_CATALOG_BTN1_PRESENT)
/*******************************************************************************
 * Function to trigger the button 1 s counter
 *
 ******************************************************************************/
static inline void trigger_button1_s_counter(void);
#endif

#if defined(SL_CATALOG_BTN0_PRESENT) && defined(SL_CATALOG_BTN1_PRESENT)
/*******************************************************************************
 * Function to trigger the button 1 l press
 *
 ******************************************************************************/
static inline void trigger_button1_l_press(void);
#endif

/*******************************************************************************
 * Function to trigger the current link report message
 *
 * @param[in] link_mask  the link mask to report to web UI
 ******************************************************************************/
static void trigger_current_link_report(uint32_t link_mask);

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
/*******************************************************************************
 * Function to trigger periodic display update functionality. It redraws
 * the display if necessarry.
 *
 * @param pxTimer  the timer handle, unused, only to be compatible with the
 *                 timer callback function signature
 ******************************************************************************/
static void trigger_display_update(TimerHandle_t pxTimer);
#endif

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
/*******************************************************************************
* Function to trigger the temperature interval report
*
******************************************************************************/
static void trigger_temperature_interval_report(void);
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
/*******************************************************************************
 * Function to trigger the temperature measurement
 *
 * @param[in] pxTimer  the timer handle, unused, only to be compatible with the
 *                     timer callback function signature
 ******************************************************************************/
static void trigger_temperature_measurement(TimerHandle_t pxTimer);
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

TaskHandle_t main_thread_handle;

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
TaskHandle_t cmd_executor_thread_handle;
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
// Register the callback functions and the context
static struct sid_event_callbacks event_callbacks = {
  .context           = NULL,
  .on_event          = on_sidewalk_event,               // Called from ISR context
  .on_msg_received   = on_sidewalk_msg_received,        // Called from sid_process()
  .on_msg_sent       = on_sidewalk_msg_sent,            // Called from sid_process()
  .on_send_error     = on_sidewalk_send_error,          // Called from sid_process()
  .on_status_changed = on_sidewalk_status_changed,      // Called from sid_process()
  .on_factory_reset  = on_sidewalk_factory_reset,       // Called from sid_process()
};

// Set configuration parameters
static struct sid_config sidewalk_config = {
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

static app_rnt_t app_rnt = {
  .sid_handle = NULL,
  .sid_state = SID_STATE_NOT_READY,
  .retry_period_in_sec = DEFAULT_RETRY_PERIOD,
  .app_state = APP_STATE_STARTING,
  .management_queue = NULL,
  .uplink_queue = NULL,
  .waited_msg_id = 0,
#if defined(SL_BLE_SUPPORTED)
  .ble_connection_request_pending = false,
#endif
#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  .display_update_timer_hnd = NULL,
#endif
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  .temperature_measure_timer_hnd = NULL,
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
};
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void main_thread(void *context)
{
  (void)context;

  sid_error_t ret = SID_ERROR_NONE;

#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  sl_gpio_t gpio = {
    .port = SL_BOARD_ENABLE_SENSOR_RHT_PORT,
    .pin = SL_BOARD_ENABLE_SENSOR_RHT_PIN,
  };
  sl_gpio_set_pin_mode(&gpio, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin(&gpio);
  sl_rht_unidriver_init(sl_i2cspm_sensor);
#endif // defined(SL_TEMPERATURE_SENSOR_EXTERNAL)

  // Queue creation for the sidewalk events
  app_rnt.management_queue = xQueueCreate(MANAGEMENT_QUEUE_LEN, sizeof(app_event_t));
  app_assert(app_rnt.management_queue != NULL, "app: management queue creation failed");

  app_rnt.uplink_queue = xQueueCreate(UPLINK_QUEUE_LEN, sizeof(app_event_t));
  app_assert(app_rnt.uplink_queue != NULL, "app: uplink queue creation failed");

  // We always work with a single link only, so the link_type is also the link_mask
  sidewalk_config.link_mask = (uint32_t) configured_link_value_to_sid_link_type(SL_SIDEWALK_COMMON_REGISTRATION_LINK);

  app_link_config(&sidewalk_config);

  // Initialize sidewalk
  ret = sid_init(&sidewalk_config, &app_rnt.sid_handle);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk init failed, link mask: %x, error: %d",
                         sidewalk_config.link_mask, ret);
    process_error();
  }

  SL_SID_LOG_APP_INFO("sidewalk inited, link mask: %x", sidewalk_config.link_mask);

  // Start the sidewalk stack
  ret = sid_start(app_rnt.sid_handle, sidewalk_config.link_mask);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk start failed, link mask: %x, error: %d",
                         sidewalk_config.link_mask, ret);
    process_error();
  }

  SL_SID_LOG_APP_INFO("sidewalk started, link mask: %x", sidewalk_config.link_mask);

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  app_rnt.display_update_timer_hnd = xTimerCreate("display_update_timer",
                                                  pdMS_TO_TICKS(1000),
                                                  pdTRUE,
                                                  (void*)0,
                                                  trigger_display_update);
  if (!app_rnt.display_update_timer_hnd) {
    SL_SID_LOG_APP_ERROR("display update timer creation failed");
    process_error();
  }
  SL_SID_LOG_APP_INFO("display update timer created");

  BaseType_t result = xTimerStart(app_rnt.display_update_timer_hnd, 0);
  if (result != pdPASS) {
    SL_SID_LOG_APP_ERROR("display update timer start failed");
    process_error();
  }
  SL_SID_LOG_APP_INFO("display update timer started");
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  // The temperature timer will be started after the link and cloud setup are complete
  // Also timer interval will be adjusted before timer start based on the current link
  app_rnt.temperature_measure_timer_hnd = xTimerCreate("temperature_timer",
                                                       pdMS_TO_TICKS(1000),
                                                       pdTRUE,
                                                       (void*)0,
                                                       trigger_temperature_measurement);
  if (!app_rnt.temperature_measure_timer_hnd) {
    SL_SID_LOG_APP_ERROR("temperature measurement timer creation failed");
    process_error();
  }
  SL_SID_LOG_APP_INFO("temperature measurement timer created");
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

  SL_SID_LOG_APP_INFO("main task started");

  while (1) {
    app_event_t app_event;

    if (xQueueReceive(app_rnt.management_queue, &app_event, 0) == pdTRUE) {
      switch (app_event.id) {
        case APP_EVENT_ID_M_SIDEWALK_PROCESS:
          // Intentionally no logging here.
          // Directly call sid_process to drive the stack.
          sid_process(app_rnt.sid_handle);
          break;
        case APP_EVENT_ID_M_BLE_CONNECTION_REQUEST:
          SL_SID_LOG_APP_INFO("management event: ble connection request");
          handle_ble_connection_request();
          break;
        case APP_EVENT_ID_M_START_NORMAL_RUNTIME:
          SL_SID_LOG_APP_INFO("management event: switching to normal runtime");
          handle_start_normal_runtime();
          break;
        case APP_EVENT_ID_M_LINK_SWITCH:
          SL_SID_LOG_APP_INFO("management event: switching link");
          handle_link_switch();
          break;
#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
        case APP_EVENT_ID_M_UPDATE_DISPLAY:
          // Intentionally no logging here.
          handle_update_display();
          break;
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
        default:
          SL_SID_LOG_APP_ERROR("unexpected management event: %d", (int)app_event.id);
          break;
      }
    } else if (app_rnt.sid_state == SID_STATE_READY &&
               xQueueReceive(app_rnt.uplink_queue, &app_event, 0) == pdTRUE) {
      switch (app_event.id) {
        case APP_EVENT_ID_U_SEND_HELLO:
          SL_SID_LOG_APP_INFO("uplink event: sending hello");
          handle_send_hello();
          break;
        case APP_EVENT_ID_U_SEND_CAPABILITIES:
          SL_SID_LOG_APP_INFO("uplink event: sending capabilities");
          handle_send_capabilities();
          break;
        case APP_EVENT_ID_U_SEND_SENSOR_DATA:
          SL_SID_LOG_APP_INFO("uplink event: sending sensor data");
          handle_send_sensor_data(&app_event);
          break;
        default:
          SL_SID_LOG_APP_ERROR("unexpected uplink event: %d", (int)app_event.id);
          break;
      }
    } else {
      (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
  }

  SL_SID_LOG_APP_ERROR("main task end reached");
  process_error();
}

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
void cmd_executor_thread(void *context)
{
  (void)(context);
  const TickType_t xDelay = 250 / portTICK_PERIOD_MS;

  // Wait for the main thread to be ready
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  while (1) {
    sl_status_t status = sl_sidewalk_cmd_executor2_execute();
    if (status != SL_STATUS_OK && status != SL_STATUS_EMPTY) {
      SL_SID_LOG_APP_ERROR("Command execution failed, error: 0x%lx", (unsigned long)status);
    }
    vTaskDelay(xDelay);
  }

  SL_SID_LOG_APP_ERROR("cmd executor thread end reached");
  process_error();
}
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
/*******************************************************************************
 * Button press callback.

 * @param[in] button Button index
 * @param[in] duration Button press duration
 ******************************************************************************/
void app_button_press_cb(uint8_t button, uint8_t duration)
{
  // Convert the button index to the actual button ID
#if defined(SL_CATALOG_BTN0_PRESENT)
  uint8_t actual_button = button;
#else
  uint8_t actual_button = 1;
#endif

  SL_SID_LOG_APP_INFO("button %u pressed (index: %u), duration: %u", actual_button, button, duration);

#if defined(SL_CATALOG_BTN0_PRESENT)
  if (actual_button == 0) {
    if (duration == APP_BUTTON_PRESS_DURATION_SHORT) {
      trigger_button0_s_counter();
    } else {
      // Consider everything not short as long press
      trigger_link_switch();
    }
  }
#endif

#if defined(SL_CATALOG_BTN1_PRESENT)
  if (actual_button == 1) {
    if (duration == APP_BUTTON_PRESS_DURATION_SHORT) {
      trigger_button1_s_counter();
    } else {
      // Consider everything not short as long press
#if defined(SL_CATALOG_BTN0_PRESENT)
      trigger_button1_l_press();
#else
      trigger_link_switch();
#endif
    }
  }
#endif
}
#endif

void trigger_link_switch(void)
{
  if (app_rnt.app_state == APP_STATE_RUNNING) {
    app_event_t app_event;
    app_event.id = APP_EVENT_ID_M_LINK_SWITCH;
    // Note: no event data to be set

    queue_app_event(app_rnt.management_queue, &app_event, false);
  } else {
    SL_SID_LOG_APP_WARNING("trigger_link_switch: not in running state yet");
  }
}

// -----------------------------------------------------------------------------
//                       Sidewalk Event Handler Functions
// -----------------------------------------------------------------------------
static void on_sidewalk_event(bool in_isr,
                              void *context)
{
  (void)(in_isr);
  (void)(context);

  trigger_sid_process();
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context)
{
  (void)(context);

  log_received_sid_msg(msg->data, msg->size, msg_desc);

  switch (app_rnt.app_state) {
    case APP_STATE_WAIT_FOR_URL_AND_READY:
      if (sl_sidewalk_web_utils_is_cloud_ready(msg->data, msg->size)) {
        app_rnt.app_state = APP_STATE_WAIT_FOR_URL;
      } else if (try_handle_url_msg(msg->data, msg->size)) {
        app_rnt.app_state = APP_STATE_WAIT_FOR_READY;
      }
      break;

    case APP_STATE_WAIT_FOR_URL:
      if (try_handle_url_msg(msg->data, msg->size)) {
        app_rnt.app_state = APP_STATE_CAPABILITIES_IN_PROGRESS;
        trigger_send_capabilities();
      }
      break;

    case APP_STATE_WAIT_FOR_READY:
      if (sl_sidewalk_web_utils_is_cloud_ready(msg->data, msg->size)) {
        app_rnt.app_state = APP_STATE_CAPABILITIES_IN_PROGRESS;
        trigger_send_capabilities();
      }
      break;

    case APP_STATE_CAPABILITIES_IN_PROGRESS:
      if (is_waited_ack(msg_desc)) {
        trigger_send_capabilities();
      }
      break;

    case APP_STATE_CAPABILITIES_LAST:
      if (is_waited_ack(msg_desc)) {
        app_event_t app_event = {
          .id = APP_EVENT_ID_M_START_NORMAL_RUNTIME
        };
        queue_app_event(app_rnt.management_queue, &app_event, false);
      }
      break;

    case APP_STATE_RUNNING: {
#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
      const char *cmd = NULL;
      size_t cmd_size = 0;
      bool is_cmd = false;
      // Note: no need to check ack here, ack does not have payload, size is always 0
      sl_status_t status = sl_sidewalk_web_utils_try_parse_cmd(&cmd,
                                                               &cmd_size,
                                                               &is_cmd,
                                                               msg->data,
                                                               msg->size);
      if (status != SL_STATUS_OK) {
        SL_SID_LOG_APP_ERROR("Failed to try-parse command, error: 0x%lx", (unsigned long)status);
        break;
      }
      if (is_cmd) {
        status = sl_sidewalk_cmd_executor2_receive(cmd, cmd_size);
        if (status != SL_STATUS_OK) {
          SL_SID_LOG_APP_ERROR("Command reception failed, error: 0x%lx", (unsigned long)status);
        }
      } else {
        if (!msg_desc->msg_desc_attr.rx_attr.is_msg_ack) {
          SL_SID_LOG_APP_WARNING("received non-command downlink message");
        }
      }
#else
      SL_SID_LOG_APP_WARNING("received downlink, but actuators and command executor are not configured");
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
      break;
    }

    default:
      // Intentionally do nothing in other states
      break;
  }
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  (void)(context);

  SL_SID_LOG_APP_INFO("sid: msg sent (link: %x, id: %u)",
                      msg_desc->link_type,
                      msg_desc->id);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  (void)(context);
  SL_SID_LOG_APP_ERROR("sid: send error (link: %x, id: %u, error: %d)",
                       msg_desc->link_type,
                       msg_desc->id,
                       (int)error);

  // Only tolerate sending errors during normal runtime.
  if (app_rnt.app_state != APP_STATE_RUNNING) {
    process_error();
  }
}

static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  (void)(context);

  SL_SID_LOG_APP_INFO("sid: status changed (reg: %u, time sync: %u, link: %lu)",
    status->detail.registration_status,
    status->detail.time_sync_status,
    status->detail.link_status_mask);

  app_rnt.sid_state = status->state;

  switch (status->state) {
    case SID_STATE_READY:
      SL_SID_LOG_APP_INFO("sid: status ready");
      app_rnt.retry_period_in_sec = get_retry_period_in_sec(status->detail.link_status_mask);
#if defined(SL_BLE_SUPPORTED)
      app_rnt.ble_connection_request_pending = false;
#endif
      if (app_rnt.app_state == APP_STATE_STARTING) {
        app_rnt.app_state = APP_STATE_WAIT_FOR_URL_AND_READY;
        // No need to check for BLE connection, as this is the ready callback itself
        app_event_t app_event = {
          .id = APP_EVENT_ID_U_SEND_HELLO
        };
        queue_app_event(app_rnt.uplink_queue, &app_event, false);
      } else if (app_rnt.app_state == APP_STATE_RUNNING) {
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
        start_temperature_timer();
#endif
        xTaskNotifyGive(main_thread_handle);
      }
      break;
    case SID_STATE_NOT_READY:
      SL_SID_LOG_APP_INFO("sid: status not ready");
      break;
    case SID_STATE_ERROR:
      SL_SID_LOG_APP_ERROR("sid: status error, error: %d", (int)sid_get_error(app_rnt.sid_handle));
      break;
    case SID_STATE_SECURE_CHANNEL_READY:
      SL_SID_LOG_APP_INFO("sid: status secure channel ready");
      break;
  }
}

static void on_sidewalk_factory_reset(void *context)
{
  (void)(context);
}

// -----------------------------------------------------------------------------
//                       Sidewalk Event Handler Functions
// -----------------------------------------------------------------------------
static inline void handle_ble_connection_request(void)
{
  sid_error_t ret = sid_ble_bcn_connection_request(app_rnt.sid_handle, true);
  switch (ret) {
    case SID_ERROR_NONE:
      SL_SID_LOG_APP_INFO("BLE connection request set");
      break;
    case SID_ERROR_ALREADY_EXISTS:
      SL_SID_LOG_APP_WARNING("BLE connection request already in progress");
      break;
    default:
      SL_SID_LOG_APP_ERROR("BLE connection request error: %d", (int)ret);
      break;
    }
}

static inline void handle_send_hello(void)
{
  char payload[SETUP_MSG_MAX_SIZE];

  sl_status_t status = sl_sidewalk_web_utils_get_initial_msg(payload, SETUP_MSG_MAX_SIZE);
  if (status != SL_STATUS_OK) {
    SL_SID_LOG_APP_ERROR("Failed to get initial message, error: 0x%lx", (unsigned long)status);
    process_error();
  }

  put_msg(payload, true, 5);

  SL_SID_LOG_APP_INFO("hello message queued");
}

static inline void handle_send_capabilities(void)
{
  char payload[SETUP_MSG_MAX_SIZE];
  bool is_completed;

  sl_status_t status = sl_sidewalk_web_utils_get_capability_msg(payload, SETUP_MSG_MAX_SIZE, &is_completed);
  if (status != SL_STATUS_OK) {
    SL_SID_LOG_APP_ERROR("Failed to get capability message, error: 0x%lx", (unsigned long)status);
    process_error();
  }

  app_rnt.waited_msg_id = put_msg(payload, true, 5);

  if (is_completed) {
    app_rnt.app_state = APP_STATE_CAPABILITIES_LAST;
  }

  SL_SID_LOG_APP_INFO("capability message queued");
}

static inline void handle_start_normal_runtime(void)
{
  trigger_current_link_report(sidewalk_config.link_mask);

  // enable link swithc, buttons, timers, etc.
#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
  app_button_press_enable();
#endif

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
  sl_sidewalk_cmd_executor2_config_t cmd_executor_config = {
    .commands = sl_sidewalk_web_utils_cmd_tbl,
    .num_of_commands = SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM,
    .arg_separator = SL_SIDEWALK_WEB_UTILS_ARG_SEPARATOR[0],
    .command_max_length = RUNTIME_MSG_STR_MAX_SIZE,
    .command_queue_size = 10
  };
  sl_status_t status = sl_sidewalk_cmd_executor2_init(&cmd_executor_config);
  if (status != SL_STATUS_OK) {
    SL_SID_LOG_APP_ERROR("Command executor initialization failed");
    process_error();
  }

  xTaskNotifyGive(cmd_executor_thread_handle);
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  start_temperature_timer();
#endif

  app_rnt.app_state = APP_STATE_RUNNING;
}

static inline void handle_link_switch(void)
{
  uint32_t next_link_mask = (uint32_t) get_next_link(sidewalk_config.link_mask);

  if (sidewalk_config.link_mask != next_link_mask) {

    trigger_current_link_report(next_link_mask);

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
    xTimerStop(app_rnt.temperature_measure_timer_hnd, 0);
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

    sid_error_t ret = SID_ERROR_NONE;
    ret = sid_deinit(app_rnt.sid_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk deinit failed, error: %d", (int)ret);
      process_error();
    }

    app_rnt.sid_handle = NULL;
    sidewalk_config.link_mask = next_link_mask;
    ret = sid_init(&sidewalk_config, &app_rnt.sid_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk init failed, error: %d", (int)ret);
      process_error();
    }

    ret = sid_start(app_rnt.sid_handle, next_link_mask);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("sidewalk start failed, error: %d", (int)ret);
      process_error();
    }

    SL_SID_LOG_APP_INFO("sidewalk switched to link: %d", next_link_mask);
  } else {
    SL_SID_LOG_APP_INFO("no other link to switch to");
  }
}

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
static inline void handle_update_display(void)
{
  if (sl_sidewalk_display_update() == true) {
    SL_SID_LOG_APP_INFO("display updated");
  }
}
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)

static inline void handle_send_sensor_data(app_event_t *app_event)
{
  // Build the message payload based on the sensor data
  char payload[RUNTIME_MSG_STR_MAX_SIZE];
  sl_status_t status = sl_sidewalk_web_utils_get_sensor_msg(payload, RUNTIME_MSG_STR_MAX_SIZE, app_event->data.capability, app_event->data.sensor_value);
  if (status != SL_STATUS_OK) {
    // Sensor message is not critical, so we can continue even if it fails
    SL_SID_LOG_APP_ERROR("Failed to get sensor message, error: 0x%lx", (unsigned long)status);
    return;
  }

  // Prepare sid_msg parameters based on the sensor data capability
  bool request_ack = true;
  uint8_t retries = 2;

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL))
  if (app_event->data.capability == SL_SIDEWALK_WEB_UTILS_C_CORE_TEMPERATURE) {
    request_ack = false;
    retries = 0;
  }
#elif defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  if (app_event->data.capability == SL_SIDEWALK_WEB_UTILS_C_ROOM_TEMPERATURE) {
    request_ack = false;
    retries = 0;
  }
#endif

  put_msg(payload, request_ack, retries);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static enum sid_link_type configured_link_value_to_sid_link_type(uint8_t link_value)
{
  enum sid_link_type ret;

  switch (link_value) {
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

static uint16_t get_retry_period_in_sec(uint32_t link_mask)
{
  uint16_t result = DEFAULT_RETRY_PERIOD;

  // Assuming that link mask always consists of a single link only
  switch (link_mask) {
    case SID_LINK_TYPE_1:
      result = 1;
      break;
    case SID_LINK_TYPE_2:
      result = 12;
      break;
    case SID_LINK_TYPE_3:
      result = 9;
      break;
    default:
      break;
  }

  return result;
}

static enum sid_link_type get_next_link(uint32_t current_link_mask)
{
  // BLE -> FSK -> CSS
  enum sid_link_type result = SID_LINK_TYPE_ANY;
  if (current_link_mask == SID_LINK_TYPE_1) {
#if defined(SL_FSK_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to FSK link");
    result = SID_LINK_TYPE_2;
#elif defined(SL_CSS_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to CSS link");
    result = SID_LINK_TYPE_3;
#else
    result = SID_LINK_TYPE_1;
#endif
  } else if (current_link_mask == SID_LINK_TYPE_2) {
#if defined(SL_CSS_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to CSS link");
    result = SID_LINK_TYPE_3;
#elif defined(SL_BLE_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to BLE link");
    result = SID_LINK_TYPE_1;
#else
    result = SID_LINK_TYPE_2;
#endif
  } else if (current_link_mask == SID_LINK_TYPE_3) {
#if defined(SL_BLE_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to BLE link");
    result = SID_LINK_TYPE_1;
#elif defined(SL_FSK_SUPPORTED)
    SL_SID_LOG_APP_INFO("switching to FSK link");
    result = SID_LINK_TYPE_2;
#else
    result = SID_LINK_TYPE_3;
#endif
  } else {
    SL_SID_LOG_APP_ERROR("link mask combining multiple links is not expected");
    process_error();
  }
  return result;
}

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static void start_temperature_timer(void)
{
  if (xTimerIsTimerActive(app_rnt.temperature_measure_timer_hnd) == pdFALSE) {
    BaseType_t result = xTimerChangePeriod(app_rnt.temperature_measure_timer_hnd,
                                           pdMS_TO_TICKS(app_rnt.retry_period_in_sec * 1000),
                                           0);
    if (result != pdPASS) {
      SL_SID_LOG_APP_ERROR("temperature measurement timer period change failed");
      return;
    }
    SL_SID_LOG_APP_INFO("temperature measurement timer period changed");

    result = xTimerStart(app_rnt.temperature_measure_timer_hnd, 0);
    if (result != pdPASS) {
      SL_SID_LOG_APP_ERROR("temperature measurement timer start failed");
      return;
    }
    SL_SID_LOG_APP_INFO("temperature measurement timer started");

    trigger_temperature_interval_report();
  }
}
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

static void queue_app_event(QueueHandle_t queue, app_event_t* app_event, bool to_front)
{
  BaseType_t ret;

  if ((bool)xPortIsInsideInterrupt()) {
    BaseType_t task_woken = pdFALSE;
    BaseType_t notify_woken = pdFALSE;

    if (to_front) {
      ret = xQueueSendToFrontFromISR(queue, app_event, &task_woken);
    } else {
      ret = xQueueSendFromISR(queue, app_event, &task_woken);
    }
    // Notify main thread only if the event was successfully queued
    // No error logging from ISR
    if (ret == pdTRUE) {
      vTaskNotifyGiveFromISR(main_thread_handle, &notify_woken);
    }
    portYIELD_FROM_ISR(task_woken || notify_woken);
  } else {
    if (to_front) {
      ret = xQueueSendToFront(queue, app_event, 0);
    } else {
      ret = xQueueSend(queue, app_event, 0);
    }
    if (ret != pdTRUE) {
      SL_SID_LOG_APP_WARNING("queue full, event %d dropped", (int)app_event->id);
    } else if (xTaskGetCurrentTaskHandle() != main_thread_handle) {
      xTaskNotifyGive(main_thread_handle);
    }
  }
}

static void check_connection(void)
{
#if defined(SL_BLE_SUPPORTED)
  if (app_rnt.sid_state != SID_STATE_READY) {
    if (sidewalk_config.link_mask & SID_LINK_TYPE_1) {
      if (!app_rnt.ble_connection_request_pending) {
        app_rnt.ble_connection_request_pending = true;
        app_event_t app_event = {
          .id = APP_EVENT_ID_M_BLE_CONNECTION_REQUEST
        };
        queue_app_event(app_rnt.management_queue, &app_event, false);
      } else {
        SL_SID_LOG_APP_WARNING("connection request already in progress");
      }
    }
  }
#endif
}

static inline void log_received_sid_msg(const void *sid_payload,
                                        size_t sid_msg_size,
                                        const struct sid_msg_desc *msg_desc)
{
  if (msg_desc->msg_desc_attr.rx_attr.is_msg_ack) {
    SL_SID_LOG_APP_INFO("sid: ack received (id: %u)", msg_desc->id);
  } else {
    char tmp_buf[sid_msg_size + 1];
    memcpy(tmp_buf, sid_payload, sid_msg_size);
    tmp_buf[sid_msg_size] = '\0';
    SL_SID_LOG_APP_INFO("sid: msg received (id: %u, size: %u, payload: %s)",
                        msg_desc->id,
                        sid_msg_size,
                        tmp_buf);
  }
}

static bool try_handle_url_msg(const void *sid_payload, size_t sid_msg_size)
{
  char parsed_url[SETUP_MSG_MAX_SIZE];
  bool url_parsed = false;
  sl_status_t status = sl_sidewalk_web_utils_try_parse_url(parsed_url,
                                                           SETUP_MSG_MAX_SIZE,
                                                           &url_parsed,
                                                           sid_payload,
                                                           sid_msg_size);
  if (status != SL_STATUS_OK) {
    SL_SID_LOG_APP_ERROR("Failed to check/parse URL, error: 0x%lx", (unsigned long)status);
    process_error();
  }
  if (url_parsed) {
    SL_SID_LOG_APP_INFO("URL: %s", parsed_url);
    #if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
    if (strlen(parsed_url) < SL_SIDEWALK_DISPLAY_MAX_STR_LENGTH) {
      sl_sidewalk_display_qr(parsed_url);
    } else {
      SL_SID_LOG_APP_WARNING("URL is too long to be displayed as QR");
    }
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  }
  return url_parsed;
}

static void process_error(void)
{
  BaseType_t result = pdPASS;
  // If error happens deinit sidewalk
  if (app_rnt.sid_handle != NULL) {
    sid_stop(app_rnt.sid_handle, sidewalk_config.link_mask);
    sid_deinit(app_rnt.sid_handle);
    app_rnt.sid_handle = NULL;
  }
  SL_SID_LOG_APP_ERROR("unrecoverable error occurred");

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
  result = xTimerDelete(app_rnt.display_update_timer_hnd, 0);
  if (result != pdPASS) {
    SL_SID_LOG_APP_ERROR("display update timer delete failed");
  }
#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
  result = xTimerDelete(app_rnt.temperature_measure_timer_hnd, 0);
  if (result != pdPASS) {
    SL_SID_LOG_APP_ERROR("temperature measurement timer delete failed");
  }
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

  (void) result;
  sid_platform_deinit();
  vTaskDelete(NULL);

  while (1) {
    // Defensive empty endless loop, should never be reached.
  }
}

static uint16_t put_msg(char *msg, bool request_ack, uint8_t retries)
{
  struct sid_msg sid_msg = {
    .data = (void *)msg,
    .size = strlen(msg)
  };

  struct sid_msg_desc sid_msg_desc = {
    .type = SID_MSG_TYPE_NOTIFY,
    .link_type = SID_LINK_TYPE_ANY,
    .link_mode = SID_LINK_MODE_CLOUD,
    .msg_desc_attr.tx_attr = {
      .request_ack = request_ack,
      .num_retries = retries,
      .ttl_in_seconds = retries * app_rnt.retry_period_in_sec,
      .additional_attr = SID_MSG_DESC_TX_ADDITIONAL_ATTRIBUTES_NONE,
    }
  };

  sid_error_t ret = sid_put_msg(app_rnt.sid_handle, &sid_msg, &sid_msg_desc);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sid_put_msg failed, error: %d", (int)ret);
    process_error();
  }

  return sid_msg_desc.id;
}

static bool is_waited_ack(const struct sid_msg_desc *msg_desc)
{
  bool result = false;
  if ((msg_desc->msg_desc_attr.rx_attr.is_msg_ack) &&
      (app_rnt.waited_msg_id == msg_desc->id)) {
    app_rnt.waited_msg_id = 0;
    result = true;
  }
  return result;
}

static inline void trigger_sid_process(void)
{
  app_event_t app_event = {
    .id = APP_EVENT_ID_M_SIDEWALK_PROCESS
  };

  // Intentionally no logging here.
  queue_app_event(app_rnt.management_queue, &app_event, false);
}

static void trigger_send_capabilities(void)
{
  check_connection();
  app_event_t app_event = {
    .id = APP_EVENT_ID_U_SEND_CAPABILITIES
  };
  queue_app_event(app_rnt.uplink_queue, &app_event, false);
}

#if defined(SL_CATALOG_BTN0_PRESENT)
static inline void trigger_button0_s_counter(void)
{
  if (app_rnt.app_state == APP_STATE_RUNNING) {
    static uint8_t btn0_press_count = 0;

    app_event_t app_event;
    app_event.id = APP_EVENT_ID_U_SEND_SENSOR_DATA;
    app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_BUTTON0_S_COUNTER;

    // Intentionally no error checking here.
    snprintf(app_event.data.sensor_value, SENSOR_DATA_STR_MAX_SIZE, "%d", ++btn0_press_count);

    check_connection();
    queue_app_event(app_rnt.uplink_queue, &app_event, false);
  } else {
    SL_SID_LOG_APP_WARNING("trigger_button0_s_counter: not in running state yet");
  }
}
#endif

#if defined(SL_CATALOG_BTN1_PRESENT)
static inline void trigger_button1_s_counter(void)
{
  if (app_rnt.app_state == APP_STATE_RUNNING) {
    static uint8_t btn1_press_count = 0;

    app_event_t app_event;
    app_event.id = APP_EVENT_ID_U_SEND_SENSOR_DATA;
    app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_BUTTON1_S_COUNTER;

    // Intentionally no error checking here.
    snprintf(app_event.data.sensor_value, SENSOR_DATA_STR_MAX_SIZE, "%d", ++btn1_press_count);

    check_connection();
    queue_app_event(app_rnt.uplink_queue, &app_event, false);
  } else {
    SL_SID_LOG_APP_WARNING("trigger_button1_s_counter: not in running state yet");
  }
}
#endif


#if defined(SL_CATALOG_BTN0_PRESENT) && defined(SL_CATALOG_BTN1_PRESENT)
static inline void trigger_button1_l_press(void)
{
  static bool btn1_press = false;
  if (app_rnt.app_state == APP_STATE_RUNNING) {
    btn1_press = !btn1_press;

    app_event_t app_event;
    app_event.id = APP_EVENT_ID_U_SEND_SENSOR_DATA;
    app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_BUTTON1_L_PRESS;

    // Intentionally no error checking here.
    snprintf(app_event.data.sensor_value, SENSOR_DATA_STR_MAX_SIZE, "%d", btn1_press);

    check_connection();
    queue_app_event(app_rnt.uplink_queue, &app_event, false);
  } else {
    SL_SID_LOG_APP_WARNING("trigger_button1_l_press: not in running state yet");
  }
}
#endif

static void trigger_current_link_report(uint32_t link_mask)
{
  app_event_t app_event;
  app_event.id = APP_EVENT_ID_U_SEND_SENSOR_DATA;
  app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_CURRENT_LINK_REPORT;

  switch (link_mask) {
    case SID_LINK_TYPE_1:
      strcpy(app_event.data.sensor_value, "BLE");
      break;
    case SID_LINK_TYPE_2:
      strcpy(app_event.data.sensor_value, "FSK");
      break;
    case SID_LINK_TYPE_3:
      strcpy(app_event.data.sensor_value, "CSS");
      break;
    default:
      SL_SID_LOG_APP_WARNING("trigger_current_link_report: invalid link mask: %lu", link_mask);
      return; // Prevent sending invalid link mask
  }

  queue_app_event(app_rnt.uplink_queue, &app_event, true);
}

#if defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)
static void trigger_display_update(TimerHandle_t pxTimer)
{
  (void)pxTimer;

  app_event_t app_event = {
    .id = APP_EVENT_ID_M_UPDATE_DISPLAY
  };

  queue_app_event(app_rnt.management_queue, &app_event, false);
}

#endif // defined(SL_CATALOG_SIDEWALK_DISPLAY_PRESENT)

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static void trigger_temperature_interval_report(void)
{
  app_event_t app_event;
  app_event.id = APP_EVENT_ID_U_SEND_SENSOR_DATA;
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL))
  app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_CORE_TEMPERATURE_INTERVAL;
#elif defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_ROOM_TEMPERATURE_INTERVAL;
#endif
  snprintf(app_event.data.sensor_value, SENSOR_DATA_STR_MAX_SIZE, "%lu", app_rnt.retry_period_in_sec);

  check_connection();
  queue_app_event(app_rnt.uplink_queue, &app_event, false);
}
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))

#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))
static void trigger_temperature_measurement(TimerHandle_t pxTimer)
{
  (void)pxTimer;

  app_event_t app_event;
  app_event.id = APP_EVENT_ID_U_SEND_SENSOR_DATA;
#if (defined(SL_TEMPERATURE_SENSOR_INTERNAL))
  app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_CORE_TEMPERATURE;
#elif defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  app_event.data.capability = SL_SIDEWALK_WEB_UTILS_C_ROOM_TEMPERATURE;
#endif

  // Get temperature sensor data
  int32_t temp_data;
  uint32_t printed_length = 0;
#if defined(SL_TEMPERATURE_SENSOR_EXTERNAL)
  uint32_t rh_data;
  sl_rht_unidriver_measure_rh_and_temp(&rh_data, &temp_data);
  printed_length = (uint32_t) snprintf(app_event.data.sensor_value, SENSOR_DATA_STR_MAX_SIZE, "%.2f", (float)((float)temp_data / 1000.0f));
#elif defined(SL_TEMPERATURE_SENSOR_INTERNAL)
  temp_data = (int32_t)TEMPDRV_GetTemp();
  printed_length = (uint32_t) snprintf(app_event.data.sensor_value, SENSOR_DATA_STR_MAX_SIZE, "%ld", temp_data);
#endif

  if (printed_length >= SENSOR_DATA_STR_MAX_SIZE) {
    SL_SID_LOG_APP_ERROR("temperature measurement value too long, truncated data is sent to the cloud");
    app_event.data.sensor_value[SENSOR_DATA_STR_MAX_SIZE - 1] = '\0';
  }

  check_connection();
  queue_app_event(app_rnt.uplink_queue, &app_event, false);
}
#endif // (defined(SL_TEMPERATURE_SENSOR_INTERNAL) || defined(SL_TEMPERATURE_SENSOR_EXTERNAL))