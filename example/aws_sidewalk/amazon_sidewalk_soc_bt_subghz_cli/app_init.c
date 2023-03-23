/***************************************************************************//**
 * @file
 * @brief app_init.c
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

#include "sl_system_init.h"
#include "app_log.h"
#include "app_assert.h"
#include "app_init.h"
#include "app_process.h"
#include "app_subghz_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "sid_pal_crypto_ifc.h"
#include "sid_pal_mfg_store_ifc.h"
#include "sid_pal_storage_kv_ifc.h"
#include "sid_pal_common_ifc.h"

#include "sx126x_config.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Main task stack size
#define MAIN_TASK_STACK_SIZE    (2048 / sizeof(configSTACK_DEPTH_TYPE))

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
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * The function is used for some basic initialization relates to the app.
 *****************************************************************************/
void app_init(void)
{
  // Initialize the Silabs system
  sl_system_init();

  app_log_info("App - amazon_sidewalk_soc_bt_subghz_cli application started\n");

  // Initialize the common PAL interfaces
  sid_error_t ret = sid_pal_common_init();
  if (ret != SID_ERROR_NONE) {
    app_log_error("Sidewalk platform common init failed, err: %d\n", ret);
  }
  app_assert(ret == SID_ERROR_NONE, "Sidewalk platform common init failed\n");

  // Initialize the Key-Value storage PAL module
  ret = sid_pal_storage_kv_init();
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - sidewalk key-value storage init failed: %d\n", ret);
  }
  app_assert(ret == SID_ERROR_NONE, "sidewalk key-value storage init failed\n");

  // Initialize PAL crypto module
  ret = sid_pal_crypto_init();
  if (ret != SID_ERROR_NONE) {
    app_log_error("App - sidewalk crypto init failed: %d\n", ret);
  }
  app_assert(ret == SID_ERROR_NONE, "sidewalk crypto init failed\n");

  // Initialize the manufacturing region. It will load the data from there if there is.
  sid_pal_mfg_store_region_t mfg_region_config;
  sid_pal_mfg_store_init(mfg_region_config);

  /* TODO: remove once SID API is in place */
  set_radio_sx126x_device_config(get_radio_cfg());

  // Application context creation
  static app_context_t app_context =
  {
    .event_queue     = NULL,
    .main_task       = NULL,
    .sidewalk_handle = NULL,
    .state           = STATE_INIT
  };

  BaseType_t status = xTaskCreate(main_task, "MAIN", MAIN_TASK_STACK_SIZE, &app_context, 1, &app_context.main_task);
  app_assert(status == pdPASS, "main task creation failed\n");

  vTaskStartScheduler();
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
