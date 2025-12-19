/***************************************************************************/ /**
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
#include <stdint.h>
#include <stdlib.h>

#include "app_process.h"
#include "app_init.h"
#include "app_assert.h"
#include "sid_api.h"
#include "app_cli.h"
#include "app_cli_settings.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_common_config.h"

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

#if defined(SL_SIDEWALK_OTA_DFU_PRESENT)
#include "sl_sidewalk_ota_dfu.h"
#endif

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
#include "sid_detect_unwanted_location_tracker.h"
#include "sid_network_address.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Maximum number Queue elements
#define MSG_QUEUE_LEN (10U)
#define MSG_CLI_QUEUE_LEN (3U)

// Unused function parameter
#define UNUSED(x) (void)(x)

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)

static const struct sid_detect_unwanted_location_tracker_accessory_info dult_info = {
  .product_data = (const uint8_t *)"sidewalk",
  .manufacturer_name = (const uint8_t *)"sidewalk",
  .model_name = (const uint8_t *)"sidewalk v1",
  .capabilities = SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_SOUND | SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_MOTION_DETECTION | SID_DETECT_UNWANTED_DETECTION_TRACKER_CAPABILITY_IDENTIFIER_LOOKUP_BLE,
  .firmware_version = SID_DETECT_UNWANTED_LOCATION_TRACKER_FIRMWARE_VERSION(SID_SDK_MAJOR_VERSION,
                                                                            SID_SDK_MINOR_VERSION,
                                                                            SID_SDK_BUILD_VERSION),
  .category = 1,
  .network_id = 3,   // IETF DULT Temporary Registry: 1-Apple, 2-Google, 3-Sidewalk
};

static void on_owner_proximity_change(enum sid_owner_proximity_state state, void *context)
{
}

static void on_non_owner_find_event(const struct sid_non_owner_find_event *event, void *context)
{
}

static void on_motion_detection_event(const struct sid_motion_detection_event *event, void *context)
{
}

static size_t on_identifier_read(uint8_t *buf, size_t len, void *context)
{
  struct sid_address address = sid_address_get_local();
  sid_error_t result = sid_address_to_raw_buffer(&address, buf, len);
  return result == SID_ERROR_NONE ? sid_address_get_size(&address) : 0;
}

static struct sid_detect_unwanted_location_tracker_event_callbacks dult_callbacks = {
  .context = NULL,
  .on_identifier_read = on_identifier_read,
  .on_motion_detection_event = on_motion_detection_event,
  .on_non_owner_find_event = on_non_owner_find_event,
  .on_owner_proximity_change = on_owner_proximity_change,
};

static struct sid_detect_unwanted_location_tracker_config config = {
  .info = &dult_info,
  .callbacks = &dult_callbacks,
};

#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Function to send an array of 0x31 with the length specified in `len`
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] message_type_str type of message to be sent: notify, get, set or response
 * @param[in] message_str content of the message to be sent
 * @param[in] link_type link type
 * @returns None
 ******************************************************************************/
static void send(app_context_t *app_context, char *message_type_str, char *message_str, char *link_type);

/*******************************************************************************
 * Method for sending sidewalk events
 *
 * @param[in] in_isr If the event shall be handled from ISR context
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void on_sidewalk_event(bool in_isr, void *context);

/*******************************************************************************
 * Callback Method for receiving sidewalk messages
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] msg The received message
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);

/*******************************************************************************
 * Callback method for the case when a sidewalk message is sent
 *
 * @param[in] msg_desc Message descriptor
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context);

/*******************************************************************************
 * Callback function if error happened during send operation
 *
 * @param[in] error The error type
 * @param[in] msg_desc Message descriptor
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void on_sidewalk_send_error(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context);

/*******************************************************************************
 * Callback Function to handle status changes in the Sidewalk context
 *
 * @param[in] status  new status
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void on_sidewalk_status_changed(const struct sid_status *status, void *context);

/*******************************************************************************
 * Callback function which is called from factory reset sidewalk event
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void on_sidewalk_factory_reset(void *context);

/*******************************************************************************
 * Function to parse the message type
 *
 * @param[in] link_type_str The link type string
 * @returns Link mask:  0 if link is not available,
                        SID_LINK_TYPE_1 for ble,
                        SID_LINK_TYPE_2 for fsk,
                        SID_LINK_TYPE_3 for css
 ******************************************************************************/
static uint8_t parse_link_type(char *link_type_str);

/*******************************************************************************
 * Function to parse the message type
 *
 * @param[in] message_type The message type string
 * @returns Message type
 ******************************************************************************/
static int8_t parse_message_type(char *message_type);

/*******************************************************************************
 * Function to initialize sidewalk stack
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] link_str Link type string
 * @returns Status of the request
 ******************************************************************************/
static sl_status_t init_sidewalk(app_context_t *app_context, char *link_str);

/*******************************************************************************
 * Function to start sidewalk stack
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] link_str Link type string
 * @returns Status of the request
 ******************************************************************************/
static sl_status_t start_sidewalk(app_context_t *app_context, char *link_str);

/*******************************************************************************
 * Function to stop sidewalk stack
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] link_str Link type string
 * @returns Status of the request
 ******************************************************************************/
static sl_status_t stop_sidewalk(app_context_t *app_context, char *link_str);

/*******************************************************************************
 * Function to deinitialize sidewalk stack
 *
 * @param[in] app_context The context which is applicable for the current application
 * @param[in] link_str Link type string
 * @returns Status of the request
 ******************************************************************************/
static sl_status_t deinit_sidewalk(app_context_t *app_context);

/*******************************************************************************
 * Function to reset sidewalk stack
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void reset_sidewalk(app_context_t *context);

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
/*******************************************************************************
 * Function to initialize dult
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns Status of the request
 ******************************************************************************/
static sl_status_t init_dult(app_context_t *context);

/*******************************************************************************
 * Function to deinitialize dult
 *
 * @param[in] app_context The context which is applicable for the current application
 * @returns Status of the request
 ******************************************************************************/
static sl_status_t deinit_dult(app_context_t *context);
#endif

/*******************************************************************************
 * Function to request connection
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
static void connection_request(app_context_t *context);

// -----------------------------------------------------------------------------
//                          Public Functions Declarations
// -----------------------------------------------------------------------------

#if defined(SL_SIDEWALK_OTA_DFU_PRESENT)
void sl_app_trigger_ota_dfu_release_buffer(void)
{
  queue_event(g_event_queue, EVENT_TYPE_SID_OTA_DFU_RELEASE_BUFFER);
  SL_SID_LOG_APP_INFO("OTA DFU release buffer event");
}
#endif

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// Link status (BLE, FSK, CSS)
link_status_t link_status;

// Last message received
struct sid_msg_desc LAST_MESSG_RCVD_DESC = { 0 };

// Queue for sending sidewalk events
QueueHandle_t g_event_queue;

// Currently initialized link
uint32_t current_init_link = 0;

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Main task
 *
 * @param[in] context The context which is applicable for the current application
 * @returns None
 ******************************************************************************/
void main_task(void *context)
{
  // Creating application context
  app_context_t *app_context = (app_context_t *)context;

  // Queue creation for the sidewalk events
  g_event_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(enum event_type));
  g_cli_event_queue = xQueueCreate(MSG_CLI_QUEUE_LEN, sizeof(app_setting_cli_queue_t));
  app_assert((g_event_queue != NULL) || (g_cli_event_queue != NULL), "queue creation failed");

  // Initialize to not ready state
  app_context->state = STATE_SIDEWALK_NOT_READY;

  // Assign queue to the application context
  app_context->event_queue = g_event_queue;

  while (1) {
    enum event_type event = EVENT_TYPE_INVALID;

    if (xQueueReceive(app_context->event_queue, &event, portMAX_DELAY) == pdTRUE) {
      switch (event) {
        case EVENT_TYPE_SIDEWALK:
          sid_process(app_context->sidewalk_handle);
          break;

        case EVENT_TYPE_SID_SEND:
          send(app_context, cli_arg_str, cli_arg_str_2, cli_arg_str_3);
          break;

        case EVENT_TYPE_SID_RESET:
          reset_sidewalk(app_context);
          break;

        case EVENT_TYPE_SID_INIT:
          if (init_sidewalk(app_context, cli_arg_str) != SL_STATUS_OK) {
            SL_SID_LOG_APP_ERROR("sidewalk initialization failed");
          }
          break;

        case EVENT_TYPE_SID_START:
          if (start_sidewalk(app_context, cli_arg_str) != SL_STATUS_OK) {
            SL_SID_LOG_APP_ERROR("sidewalk start failed");
          }
          break;

        case EVENT_TYPE_SID_STOP:
          if (stop_sidewalk(app_context, cli_arg_str) != SL_STATUS_OK) {
            SL_SID_LOG_APP_ERROR("sidewalk stop failed");
          }
          break;

        case EVENT_TYPE_SID_DEINIT:
          if (deinit_sidewalk(app_context) != SL_STATUS_OK) {
            SL_SID_LOG_APP_ERROR("sidewalk deinitialization failed");
          }
          break;

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
        case EVENT_TYPE_DULT_INIT:
          if (init_dult(app_context) != SL_STATUS_OK) {
            SL_SID_LOG_APP_ERROR("dult initialization failed");
          }
          break;

        // case EVENT_TYPE_DULT_STATUS:
        // if (stop_sidewalk(app_context) != SL_STATUS_OK) {
        //   SL_SID_LOG_APP_ERROR("dult status failed");
        // }
        // break;

        case EVENT_TYPE_DULT_DEINIT:
          if (deinit_dult(app_context) != SL_STATUS_OK) {
            SL_SID_LOG_APP_ERROR("dult deinitialization failed");
          }
          break;
#endif

        case EVENT_TYPE_SID_GET_CSS_DEV_PROF_ID:
          get_sidewalk_css_dev_prof_id(app_context);
          break;

        case EVENT_TYPE_SID_SET_CSS_DEV_PROF_ID:
          set_sidewalk_css_dev_prof_id(app_context);
          break;

        case EVENT_TYPE_SID_GET_FSK_DEV_PROF_ID:
          get_sidewalk_fsk_dev_prof_id(app_context);
          break;

        case EVENT_TYPE_SID_SET_FSK_DEV_PROF_ID:
          set_sidewalk_fsk_dev_prof_id(app_context);
          break;

        case EVENT_TYPE_SID_SET_DEV_PROF_ID:
          set_sidewalk_dev_prof_id(app_context, cli_arg_uint8_t);
          break;

        case EVENT_TYPE_SID_GET_DEV_PROF_RX_WIN_CNT:
          get_sidewalk_dev_prof_rx_win_cnt(app_context);
          break;

        case EVENT_TYPE_SID_SET_DEV_PROF_RX_WIN_CNT:
          set_sidewalk_dev_prof_rx_win_cnt(app_context, (uint16_t)cli_arg_int16_t);
          break;

        case EVENT_TYPE_SID_GET_DEV_PROF_RX_INTERV_MS:
          get_sidewalk_dev_prof_rx_interv_ms(app_context);
          break;

        case EVENT_TYPE_SID_SET_DEV_PROF_RX_INTERV_MS:
          set_sidewalk_dev_prof_rx_interv_ms(app_context, cli_arg_uint16_t);
          break;

        case EVENT_TYPE_SID_GET_DEV_PROF_WAKEUP_TYPE:
          get_sidewalk_dev_prof_wakeup_type(app_context);
          break;

        case EVENT_TYPE_SID_SET_DEV_PROF_WAKEUP_TYPE:
          set_sidewalk_dev_prof_wakeup_type(app_context, cli_arg_uint8_t);
          break;

        case EVENT_TYPE_GET_TIME:
          get_sidewalk_time(app_context);
          break;

        case EVENT_TYPE_GET_STATUS:
          get_sidewalk_status(app_context);
          break;

        case EVENT_TYPE_GET_MTU_BLE:
          get_sidewalk_mtu(app_context, SID_LINK_TYPE_1);
          break;

        case EVENT_TYPE_GET_MTU_CSS:
          get_sidewalk_mtu(app_context, SID_LINK_TYPE_3);
          break;

        case EVENT_TYPE_GET_MTU_FSK:
          get_sidewalk_mtu(app_context, SID_LINK_TYPE_2);
          break;

        case EVENT_TYPE_GET_LINK_CONNECTION_POLICY:
          get_link_connection_policy(app_context);
          break;

        case EVENT_TYPE_SET_LINK_CONNECTION_POLICY:
          set_link_connection_policy(app_context, cli_arg_uint8_t);
          break;

        case EVENT_TYPE_GET_MULTI_LINK_POLICY:
          get_multi_link_policy(app_context);
          break;

        case EVENT_TYPE_SET_MULTI_LINK_POLICY:
          set_multi_link_policy(app_context, cli_arg_uint8_t);
          break;

        case EVENT_TYPE_GET_AUTO_CONNECT_PARAMS:
          get_auto_connect_params(app_context);
          break;

        case EVENT_TYPE_SET_AUTO_CONNECT_PARAMS:
          set_auto_connect_params(app_context, cli_arg_sid_link_auto_connect_params);
          break;

        case EVENT_TYPE_SID_BLE_CONNECTION_REQUEST:
          connection_request(app_context);
          break;

#if defined(SL_SIDEWALK_OTA_DFU_PRESENT)
        case EVENT_TYPE_SID_OTA_DFU_INIT_REQUEST:
          sl_sid_ota_dfu_init(app_context->sidewalk_handle);
          break;

        case EVENT_TYPE_SID_OTA_DFU_DEINIT_REQUEST:
          sl_sid_ota_dfu_deinit(app_context->sidewalk_handle);
          break;

        case EVENT_TYPE_SID_OTA_DFU_CANCEL_REQUEST:
          sl_sid_ota_dfu_cancel(app_context->sidewalk_handle);
          break;

        case EVENT_TYPE_SID_OTA_DFU_STAT_REQUEST:
          sl_sid_ota_dfu_stat(app_context->sidewalk_handle);
          break;

        case EVENT_TYPE_SID_OTA_DFU_PARAM_REQUEST:
          sl_sid_ota_dfu_param(app_context->sidewalk_handle);
          break;

        case EVENT_TYPE_SID_OTA_DFU_MIN_SCRATCH_BUF_SIZE_REQUEST:
          sl_sid_ota_dfu_min_scratch_buf_size();
          break;

        case EVENT_TYPE_SID_OTA_DFU_RELEASE_BUFFER:
          sl_sid_ota_dfu_release_buffer();
          break;
#endif

        default:
          SL_SID_LOG_APP_ERROR("unexpected event: %d", (int)event);
          break;
      }
    }
  }

  // should never reach here
  SL_SID_LOG_APP_ERROR("unrecoverable error occurred");
  sid_platform_deinit();
  vTaskDelete(NULL);
}

/*******************************************************************************
 * Issue a queue event.
 *
 * @param[in] queue The queue handle which will be used ofr the event
 * @param[in] event The event to be sent
 * @param[in] data Data to be sent
 * @returns None
 ******************************************************************************/
void queue_event(QueueHandle_t queue, enum event_type event)
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

#if defined(SL_CATALOG_SIMPLE_BUTTON_PRESENT)
/*******************************************************************************
 * Button handler callback
 * @param[in] handle button handler
 * @returns None
 ******************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      // Button0 action
    } else if (&sl_button_btn1 == handle) {
      // Button1 action
    }
  }
}
#endif

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void on_sidewalk_event(bool in_isr, void *context)
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
  // Copy last received message sturct before it is freed by the sidewalk stack.
  // Used for cli utility, to print rssi and snr of the last received message.
  memcpy(&LAST_MESSG_RCVD_DESC, msg_desc, sizeof(struct sid_msg_desc));
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

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc, void *context)
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

static void on_sidewalk_status_changed(const struct sid_status *status, void *context)
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

    default:
      // Invalid state, nothing to do
      break;
  }

  SL_SID_LOG_APP_INFO("registration status: %u, time sync: %u, link: %lu",
                      status->detail.registration_status,
                      status->detail.time_sync_status,
                      status->detail.link_status_mask);

  app_context->link_status.link_mask = status->detail.link_status_mask;
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
  SL_SID_LOG_APP_INFO("device factory reset");
  // This is the callback function of the factory reset and as the last step a reset is applied.
  NVIC_SystemReset();
}

static uint8_t parse_link_type(char *link_type_str)
{
  uint8_t link_type = 0;

  if (strstr(link_type_str, "ble") != NULL) {
#if defined(SL_BLE_SUPPORTED)
    link_type |= SID_LINK_TYPE_1;
#else
    SL_SID_LOG_APP_ERROR("BLE link not available");
    link_type = 0;
    goto cleanup;
#endif
  }

  if (strstr(link_type_str, "fsk") != NULL) {
#if defined(SL_FSK_SUPPORTED)
    link_type |= SID_LINK_TYPE_2;
#else
    SL_SID_LOG_APP_ERROR("FSK link not available");
    link_type = 0;
    goto cleanup;
#endif
  }

  if (strstr(link_type_str, "css") != NULL) {
#if defined(SL_CSS_SUPPORTED)
    link_type |= SID_LINK_TYPE_3;
#else
    SL_SID_LOG_APP_ERROR("CSS link not available");
    link_type = 0;
    goto cleanup;
#endif
  }

  cleanup:
  return link_type;
}

static int8_t parse_message_type(char *message_type)
{
  if (strcmp(message_type, "notify") == 0) {
    return SID_MSG_TYPE_NOTIFY;
  }

  if (strcmp(message_type, "get") == 0) {
    return SID_MSG_TYPE_GET;
  }

  if (strcmp(message_type, "set") == 0) {
    return SID_MSG_TYPE_SET;
  }

  if (strcmp(message_type, "response") == 0) {
    return SID_MSG_TYPE_RESPONSE;
  }

  return -1;
}

static void connection_request(app_context_t *context)
{
#if defined(SL_BLE_SUPPORTED)
  if (context->state == STATE_SIDEWALK_READY) {
    SL_SID_LOG_APP_WARNING("BLE connection is already established");
  } else {
    sid_error_t ret = sid_ble_bcn_connection_request(context->sidewalk_handle, true);
    if (ret == SID_ERROR_NONE) {
      SL_SID_LOG_APP_INFO("BLE connection request set");
      context->ble_connection_status = true;
    } else {
      SL_SID_LOG_APP_ERROR("BLE connection request failed, error: %d", (int)ret);
    }
  }
#else
  (void)context;
  SL_SID_LOG_APP_ERROR("BLE link not supported on this platform");
#endif
}

static void send(app_context_t *app_context, char *message_type_str, char *message_str, char *link_type)
{
  int8_t message_type = parse_message_type(message_type_str);

  if (message_type < 0) {
    SL_SID_LOG_APP_ERROR("wrong message type: %s", message_type_str);
    return;
  }

  SL_SID_LOG_APP_INFO("sending %s", message_str);

  struct sid_msg msg = {
    .data = (uint8_t *)message_str,
    .size = strlen(message_str)
  };

  // The descriptor is cleared and then only partially initialized which is intentional
  struct sid_msg_desc desc;
  memset(&desc, 0, sizeof(desc));
  desc.type = (uint8_t)message_type;
  desc.link_type = *link_type == 0 ? app_context->sid_cfg.link_mask : parse_link_type(link_type);
  desc.link_mode = SID_LINK_MODE_CLOUD;

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
  }

  app_context->counter++;
}

static sl_status_t init_sidewalk(app_context_t *app_context, char *link_str)
{
  app_context->sid_event_cb = (struct sid_event_callbacks){
    .context = app_context,
    .on_event = on_sidewalk_event,                     // Called from ISR context
    .on_msg_received = on_sidewalk_msg_received,       // Called from sid_process()
    .on_msg_sent = on_sidewalk_msg_sent,               // Called from sid_process()
    .on_send_error = on_sidewalk_send_error,           // Called from sid_process()
    .on_status_changed = on_sidewalk_status_changed,   // Called from sid_process()
    .on_factory_reset = on_sidewalk_factory_reset,     // Called from sid_process()
  };

  app_context->sid_cfg = (struct sid_config){
    .link_mask = parse_link_type(link_str),
    .dev_ch = {
      .type = SID_END_DEVICE_TYPE_STATIC,
      .power_type = SID_END_DEVICE_POWERED_BY_LINE_POWER_ONLY,
      .qualification_id = 0x0002,
    },
    .callbacks = &app_context->sid_event_cb,
    .link_config = NULL,
    .sub_ghz_link_config = NULL
  };

  if (app_context->sid_cfg.link_mask == 0) {
    // Issued link is not available on current platform
    // Ignore command but don't return error as this will cause main task to be deleted.
    SL_SID_LOG_APP_WARNING("chosen link not available on this platform");
    return SL_STATUS_OK;
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

#if defined(SV_ENABLED)
  SL_SID_LOG_APP_INFO("Secure Vault is enabled");
#else
  SL_SID_LOG_APP_INFO("Secure Vault is disabled");
#endif

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
  SL_SID_LOG_APP_INFO("DULT is enabled");
  SL_SID_LOG_APP_WARNING("DULT - Only issue 'dult init' after 'sid init ble' and before 'sid start ble'");
  SL_SID_LOG_APP_WARNING("DULT - Only issue 'dult deinit' after 'sid stop ble' and before 'sid deinit'");
#endif

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  app_context->sid_cfg.sub_ghz_link_config = app_get_sub_ghz_config();
#endif

#if defined(SL_BLE_SUPPORTED)
  app_context->sid_cfg.link_config = app_get_ble_config();
  app_context->ble_connection_status = false;
#endif

  struct sid_handle *tmp_sidewalk_handle;

  sid_error_t ret = sid_init(&app_context->sid_cfg, &tmp_sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    if (ret != SID_ERROR_ALREADY_INITIALIZED) {
      // reset context sidewalk_handle
      app_context->sidewalk_handle = NULL;
      app_context->sid_cfg.link_mask = 0;
      SL_SID_LOG_APP_ERROR("sidewalk initialization failed, error: %d", (int)ret);
    } else {
      // here we want to preserve the context sidewalk_handle from previous initialized link
      SL_SID_LOG_APP_ERROR("sidewalk already initializated");
    }
    return SL_STATUS_FAIL;
  }
  SL_SID_LOG_APP_INFO("sidewalk initializated");
  // update the  context sidewalk_handle with the one returned by sid_init
  app_context->sidewalk_handle = tmp_sidewalk_handle;
  current_init_link = app_context->sid_cfg.link_mask;

#if defined(SL_SIDEWALK_OTA_DFU_PRESENT)
  SL_SID_LOG_APP_INFO("sidewalk OTA DFU over BLE enabled");
#endif

  return SL_STATUS_OK;
}

static sl_status_t start_sidewalk(app_context_t *app_context, char *link_str)
{
  uint8_t link_mask = parse_link_type(link_str);
  if (link_mask == 0) {
    // Issued link is not available on current platform
    // Ignore command but don't return error as this will cause main task to be deleted.
    SL_SID_LOG_APP_ERROR("chosen link not available on this platform");
    return SL_STATUS_OK;
  }

  sid_error_t ret = sid_start(app_context->sidewalk_handle, link_mask);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk start failed, link mask: %x, error: %d", (int)link_mask, (int)ret);
    app_context->sidewalk_handle = NULL;
    app_context->sid_cfg.link_mask = 0;

    return SL_STATUS_FAIL;
  } else {
    SL_SID_LOG_APP_INFO("sidewalk started, link mask: %x", (int)link_mask);
  }

  return SL_STATUS_OK;
}

static sl_status_t stop_sidewalk(app_context_t *app_context, char *link_str)
{
  uint8_t link_mask = parse_link_type(link_str);
  if (link_mask == 0) {
    // Issued link is not available on current platform
    // Ignore the command but don't return error as this will cause main task to be deleted.
    SL_SID_LOG_APP_ERROR("chosen link not available on this platform");
    return SL_STATUS_OK;
  }

  sid_error_t ret = sid_stop(app_context->sidewalk_handle, link_mask);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk stop failed, error: %d", (int)ret);
    app_context->sidewalk_handle = NULL;
    app_context->sid_cfg.link_mask = 0;

    return SL_STATUS_FAIL;
  } else {
    SL_SID_LOG_APP_INFO("sidewalk stopped");
  }

  return SL_STATUS_OK;
}

static sl_status_t deinit_sidewalk(app_context_t *app_context)
{
  sid_error_t ret = sid_deinit(app_context->sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk deinitialization failed, error: %d", (int)ret);
    app_context->sidewalk_handle = NULL;
    app_context->sid_cfg.link_mask = 0;

    return SL_STATUS_FAIL;
  } else {
    SL_SID_LOG_APP_INFO("sidewalk deinitialized");
  }

  return SL_STATUS_OK;
}

static void reset_sidewalk(app_context_t *context)
{
  // Check the state of the connection to sidewalk
  // Must be established for the API to send de-registration to cloud
  if (context->state == STATE_SIDEWALK_READY) {
    sid_error_t ret = sid_set_factory_reset(context->sidewalk_handle);
    if (ret != SID_ERROR_NONE) {
      SL_SID_LOG_APP_ERROR("factory reset failed, error: %d", (int)ret);
      vTaskDelay(pdMS_TO_TICKS(200));
      NVIC_SystemReset();
    } else {
      SL_SID_LOG_APP_INFO("factory reset request accepted");
    }
  } else {
    SL_SID_LOG_APP_WARNING("sidewalk is not ready");
  }
}

#if defined(SID_SDK_INTERNAL_CONFIG_ENABLE_DULT_QA)
static sl_status_t init_dult(app_context_t *context)
{
  sid_error_t ret = sid_detect_unwanted_location_tracker_init(&config, context->sidewalk_handle);
  if (ret == SID_ERROR_ALREADY_INITIALIZED) {
    SL_SID_LOG_APP_ERROR("dult already initialized");
  }
  /* Not reset sidewalk handle if 'dult init' is issued during sidewalk started state */
  else if (ret == SID_ERROR_INVALID_STATE) {
    SL_SID_LOG_APP_ERROR("dult cannot be initialized during sidewalk started state, error: %d", (int)ret);
  }
  else if (ret != SID_ERROR_NONE)
  {
    SL_SID_LOG_APP_ERROR("dult initialization failed, error: %d", (int)ret);
    context->sidewalk_handle = NULL;
    context->sid_cfg.link_mask = 0;
  }
  else
  {
    SL_SID_LOG_APP_INFO("dult initialized");
  }

  return (ret == SID_ERROR_NONE) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

static sl_status_t deinit_dult(app_context_t *context)
{
  sid_error_t ret = sid_detect_unwanted_location_tracker_deinit(context->sidewalk_handle);

  /* Not reset sidewalk handle if 'dult deinit' is issued during sidewalk started state */
  if(ret == SID_ERROR_INVALID_STATE) {
    SL_SID_LOG_APP_ERROR("dult is not initialized or");
    SL_SID_LOG_APP_ERROR("dult cannot be deinitialized during sidewalk started state, error: %d", (int)ret);
  } else if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("dult deinitialization failed, error: %d", (int)ret);
    context->sidewalk_handle = NULL;
    context->sid_cfg.link_mask = 0;
  }
  else
  {
    SL_SID_LOG_APP_INFO("dult deinitialized");
  }

  return (ret == SID_ERROR_NONE) ? SL_STATUS_OK : SL_STATUS_FAIL;
}
#endif
