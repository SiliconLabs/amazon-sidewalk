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
#include "app_assert.h"
#include "app_init.h"
#include "app_process.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sid_pal_common_ifc.h"
#include "sid_api.h"
#include "sl_main_init.h"
#include "sl_sidewalk_common_config.h"
#include "sl_sidewalk_utils.h"
#include "sl_sidewalk_log_app.h"

#if defined(SL_CATALOG_SIDEWALK_DEVICE_BACKUP_PRESENT)
#include "sl_sidewalk_device_backup.h"
#endif

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

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
  SL_SID_LOG_APP_INFO("OOB application started");
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

  platform_parameters_t platform_parameters = {
#if (SL_CSS_SUPPORTED || SL_FSK_SUPPORTED)
    .platform_init_parameters.radio_cfg = get_radio_cfg(),
#endif
  };

  sid_error_t ret_code = sid_platform_init(&platform_parameters);
  app_assert(ret_code == SID_ERROR_NONE, "platform init failed: %d", ret_code);
  SL_SID_LOG_APP_INFO("platform initialized");

#if defined(SL_CATALOG_SIDEWALK_DEVICE_BACKUP_PRESENT)
  sl_sidewalk_device_backup_handle_backup_restore();
#endif

  BaseType_t status = xTaskCreate(main_thread,
                                  "MAIN",
                                  MAIN_TASK_STACK_SIZE,
                                  NULL,
                                  10,
                                  NULL);
  app_assert(status == pdPASS, "main task creation failed: %ld", status);
  SL_SID_LOG_APP_INFO("main task created");

  status = xTaskCreate(oob_msg_receiver_thread,
                       "OOBRECEIVER",
                       OOC_RCV_TASK_STACK_SIZE,
                       NULL,
                       5,
                       NULL);
  app_assert(status == pdPASS, "OOB receiver task creation failed: %ld", status);
  SL_SID_LOG_APP_INFO("OOB receiver task created");
}
