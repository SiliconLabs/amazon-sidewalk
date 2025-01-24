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
#include "sl_system_init.h"
#include "sl_sidewalk_log_app.h"
#include "app_assert.h"
#include "app_init.h"
#include "app_process.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sid_pal_common_ifc.h"
#include "sid_pal_mfg_store_ifc.h"
#include "sid_api.h"
#include "sl_system_kernel.h"
#include "sl_sidewalk_common_config.h"
#include "sl_sidewalk_utils.h"

#include "app_radio_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

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
  // Initialize the Silabs system
  sl_system_init();

  SL_SID_LOG_APP_INFO("empty project application started");
  SL_SID_LOG_APP_INFO("sidewalk stack version %s", SID_SDK_VERSION_STRING);
  SL_SID_LOG_APP_INFO("silabs sidewalk extension version %s", SL_SIDEWALK_EXT_VER_STR);

  char smsn_str[SL_SIDEWALK_UTILS_SMSN_STR_LENGTH];
  memset(smsn_str, 0, sizeof(smsn_str));
  sl_sidewalk_utils_get_smsn_as_str(smsn_str, SL_SIDEWALK_UTILS_SMSN_STR_LENGTH);
  SL_SID_LOG_APP_INFO("sidewalk SMSN: %s", smsn_str);

  char sidewalk_id_str[SL_SIDEWALK_UTILS_SIDEWALK_ID_STR_LENGTH];
  memset(sidewalk_id_str, 0, sizeof(sidewalk_id_str));
  sl_sidewalk_utils_get_sidewalk_id_as_str(sidewalk_id_str, SL_SIDEWALK_UTILS_SIDEWALK_ID_STR_LENGTH);
  SL_SID_LOG_APP_INFO("sidewalk ID: %s", sidewalk_id_str);

  platform_parameters_t platform_parameters = {0};

  app_radio_config(&platform_parameters);

  sid_error_t ret_code = sid_platform_init(&platform_parameters);
  if (ret_code != SID_ERROR_NONE) {
    SL_SID_LOG_APP_ERROR("platform initialization failed, error: %d", ret_code);
    app_assert(ret_code == SID_ERROR_NONE, "platform initialization failed, error: %d", ret_code);
  }
  SL_SID_LOG_APP_INFO("platform initialized");

  BaseType_t status = xTaskCreate(main_thread,
                                  "MAIN",
                                  MAIN_TASK_STACK_SIZE,
                                  NULL,
                                  1,
                                  NULL);
  if (status != pdPASS) {
    SL_SID_LOG_APP_ERROR("main task creation failed, error: %d", (int)status);
    app_assert(status == pdPASS, "main task creation failed, error: %d", (int)status);
  }
  SL_SID_LOG_APP_INFO("main task created");

  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
}
