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

#include "sl_component_catalog.h"
#include "app_init.h"
#include "app_process.h"
#include "app_assert.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_utils.h"
#include "sid_api.h"
#include "sl_sidewalk_common_config.h"
#include "app_link_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define UNUSED(x) (void)(x)
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

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

struct sid_handle *sidewalk_handle = NULL;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void main_thread(void *context)
{
  // Creating application context
  (void)context;

  // Register the callback functions and the context
  struct sid_event_callbacks event_callbacks =
  {
    .context           = NULL,
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
    .link_mask = link_type_to_link_mask(SL_SIDEWALK_COMMON_REGISTRATION_LINK),
    .dev_ch = {
      .type = SID_END_DEVICE_TYPE_STATIC,
      .power_type = SID_END_DEVICE_POWERED_BY_LINE_POWER_ONLY,
      .qualification_id = 0x0002,
    },
    .callbacks = &event_callbacks,
    .link_config = NULL,
    .sub_ghz_link_config = NULL,
  };

  app_link_config(&config);

  // Initialise sidewalk
  sid_error_t ret = sid_init(&config, &sidewalk_handle);
  if (ret != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("sidewalk initialization failed, link mask: %x, error: %d", (int)config.link_mask, (int)ret);
    goto error;
  }

  SL_SID_LOG_APP_INFO("main task started");

  while (1) {
    // Add your code
  }

  error:
  // If error happens deinit sidewalk
  if (sidewalk_handle != NULL) {
    sid_stop(sidewalk_handle, config.link_mask);
    sid_deinit(sidewalk_handle);
    sidewalk_handle = NULL;
  }
  SL_SID_LOG_APP_ERROR("unrecoverable error occurred");

  sid_platform_deinit();
  vTaskDelete(NULL);
}

// -----------------------------------------------------------------------------
//                       Sidewalk Event Handler Functions
// -----------------------------------------------------------------------------
static void on_sidewalk_event(bool in_isr,
                              void *context)
{
  UNUSED(in_isr);
  UNUSED(context);
}

static void on_sidewalk_msg_received(const struct sid_msg_desc *msg_desc,
                                     const struct sid_msg *msg,
                                     void *context)
{
  UNUSED(msg_desc);
  UNUSED(msg);
  UNUSED(context);
}

static void on_sidewalk_msg_sent(const struct sid_msg_desc *msg_desc,
                                 void *context)
{
  UNUSED(msg_desc);
  UNUSED(context);
}

static void on_sidewalk_send_error(sid_error_t error,
                                   const struct sid_msg_desc *msg_desc,
                                   void *context)
{
  UNUSED(error);
  UNUSED(msg_desc);
  UNUSED(context);
}

static void on_sidewalk_status_changed(const struct sid_status *status,
                                       void *context)
{
  UNUSED(status);
  UNUSED(context);
}

static void on_sidewalk_factory_reset(void *context)
{
  UNUSED(context);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

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
