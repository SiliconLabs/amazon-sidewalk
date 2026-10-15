/***************************************************************************//**
 * @file
 * @brief app_init.c
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

#include "sl_sidewalk_log_app.h"
#include "app_assert.h"
#include "app_process.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sid_pal_common_ifc.h"
#include "sid_pal_mfg_store_ifc.h"
#include "sid_api.h"
#include "sl_main_init.h"
#include "sl_sidewalk_common_config.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_web_utils_gen.h"
#include "app_radio_config.h"

// -----------------------------------------------------------------------------
//                          External Global Variables
// -----------------------------------------------------------------------------
extern TaskHandle_t main_thread_handle;

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
extern TaskHandle_t cmd_executor_thread_handle;
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Based on the main goal of Cloud Light application, and for complexity reasons, at least one sensor is required.
// (Even if the support component allows zero capabilities.)
#if SL_SIDEWALK_WEB_UTILS_SENSOR_NUM == 0
#error "Cloud Light application requires at least one sensor"
#endif // SL_SIDEWALK_WEB_UTILS_SENSOR_NUM == 0

// -----------------------------------------------------------------------------
//                          Public Function Prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * The function is used for some basic initialization relates to the app.
 *****************************************************************************/
void app_init(void)
{
  platform_parameters_t platform_parameters = { 0 };
  sid_error_t ret_code;
  BaseType_t status;
  char buff[(SL_SIDEWALK_UTILS_SMSN_STR_LENGTH > SL_SIDEWALK_UTILS_SIDEWALK_ID_STR_LENGTH)
             ? SL_SIDEWALK_UTILS_SMSN_STR_LENGTH
             : SL_SIDEWALK_UTILS_SIDEWALK_ID_STR_LENGTH] = { 0 };

  SL_SID_LOG_APP_INFO("cloud light application started");
  SL_SID_LOG_APP_INFO("sidewalk stack version %s", SID_SDK_VERSION_STRING);
  SL_SID_LOG_APP_INFO("silabs sidewalk extension version %s", SL_SIDEWALK_EXT_VER_STR);

  sl_sidewalk_utils_get_smsn_as_str(buff, SL_SIDEWALK_UTILS_SMSN_STR_LENGTH);
  SL_SID_LOG_APP_INFO("sidewalk SMSN: %s", buff);

  sl_sidewalk_utils_get_sidewalk_id_as_str(buff, SL_SIDEWALK_UTILS_SIDEWALK_ID_STR_LENGTH);
  SL_SID_LOG_APP_INFO("sidewalk ID: %s", buff);

  app_radio_config(&platform_parameters);

  ret_code = sid_platform_init(&platform_parameters);
  app_assert(ret_code == SID_ERROR_NONE, "platform init failed: %d", ret_code);
  SL_SID_LOG_APP_INFO("platform initialized");

  status = xTaskCreate(main_thread,
                       "MAIN",
                       MAIN_TASK_STACK_SIZE,
                       NULL,
                       1,
                       &main_thread_handle);
  app_assert(status == pdPASS, "main task creation failed: %ld", status);
  SL_SID_LOG_APP_INFO("main task created");

#if SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
  status = xTaskCreate(cmd_executor_thread,
                       "CMDEXECUTOR",
                       CMD_EXECUTOR_TASK_STACK_SIZE,
                       NULL,
                       5,
                       &cmd_executor_thread_handle);
  app_assert(status == pdPASS, "cmd executor task creation failed: %ld", status);
  SL_SID_LOG_APP_INFO("cmd executor task created");
#endif // SL_SIDEWALK_WEB_UTILS_ACTUATOR_NUM > 0
}
