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
#include "FreeRTOS.h"
#include "task.h"
#include "sid_pal_common_ifc.h"
#include "sid_api.h"
#include "sl_system_kernel.h"

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Main task stack size
#if defined(SL_EFR32XG24_LITE_SUPPORTED)
#define MAIN_TASK_STACK_SIZE    (2048 / sizeof(configSTACK_DEPTH_TYPE))
#else
#define MAIN_TASK_STACK_SIZE    (4096 / sizeof(configSTACK_DEPTH_TYPE))
#endif
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

  app_log_info("app: app started\n");

  platform_parameters_t platform_parameters = {
#if defined(SL_RADIO_NATIVE)
    .platform_init_parameters.radio_cfg = (radio_efr32xgxx_device_config_t *)get_radio_cfg(),
#elif defined(SL_RADIO_EXTERNAL)
    .platform_init_parameters.radio_cfg = (radio_sx126x_device_config_t *)get_radio_cfg(),
#endif
  };

  sid_error_t ret_code = sid_platform_init(&platform_parameters);
  if (ret_code != SID_ERROR_NONE) {
    app_log_error("app: sid platform init err: %d\n", ret_code);
  }
  app_assert(ret_code == SID_ERROR_NONE, "app: sid platform init failed\n");

  // Application context creation
  static app_context_t app_context =
  {
    .event_queue     = NULL,
    .main_task       = NULL,
    .sidewalk_handle = NULL,
    .state           = STATE_INIT
  };

  BaseType_t status = xTaskCreate(main_task,
                                  "MAIN",
                                  MAIN_TASK_STACK_SIZE,
                                  &app_context,
                                  1,
                                  &app_context.main_task);
  app_assert(status == pdPASS, "app: main task creation failed\n");

  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
}
