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
#include "app_button_press.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sid_pal_crypto_ifc.h"
#include "sid_pal_mfg_store_ifc.h"
#include "sid_pal_storage_kv_ifc.h"
#include "sid_pal_common_ifc.h"
#include "sl_system_kernel.h"

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#include "app_subghz_config.h"
#endif

#if defined(SIDEWALK_POWER_CONSUMPTION_WORKAROUND)
#include "peripheral_sysrtc.h"
#include "em_cmu.h"
#endif

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

#if defined(SIDEWALK_POWER_CONSUMPTION_WORKAROUND)
  // Initialize the SYSRTC module, RAIL will use it internally.
  // Pre-initialized by the system init by default but not if sleep timer is using BURTC as peripheral.
  // BURTC is used as peripheral instead of SYSRTC because of a power
  // manager issue found which cause higher power consumption.

  sl_sysrtc_config_t sysrtc_config = SYSRTC_CONFIG_DEFAULT;
  CMU_ClockEnable(cmuClock_SYSRTC, true);
  sl_sysrtc_init(&sysrtc_config);
  sl_sysrtc_enable();
#endif

  app_button_press_enable();

  app_log_info("app: sid_subghz application started");
  // Initialize the common PAL interfaces
  sid_error_t ret = sid_pal_common_init();
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: sidewalk platform common init failed, err: %d", ret);
  }
  app_assert(ret == SID_ERROR_NONE, "sidewalk platform common init failed");

  // Initialize the Key-Value storage PAL module
  ret = sid_pal_storage_kv_init();
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: sidewalk key-value storage init failed: %d", ret);
  }
  app_assert(ret == SID_ERROR_NONE, "sidewalk key-value storage init failed");

  // Initialize PAL crypto module
  ret = sid_pal_crypto_init();
  if (ret != SID_ERROR_NONE) {
    app_log_error("app: sidewalk crypto init failed: %d", ret);
  }
  app_assert(ret == SID_ERROR_NONE, "sidewalk crypto init failed");

  // Initialize the manufacturing region. It will load the data from there if there is.
  sid_pal_mfg_store_region_t mfg_region_config;
  sid_pal_mfg_store_init(mfg_region_config);

#if defined(SL_RADIO_NATIVE)
  set_radio_efr32xgxx_device_config(get_radio_cfg());
#else
  set_radio_sx126x_device_config(get_radio_cfg());
#endif

  BaseType_t status = xTaskCreate(main_thread,
                                  "MAIN",
                                  MAIN_TASK_STACK_SIZE,
                                  NULL,
                                  1,
                                  NULL);
  app_assert(status == pdPASS, "main task creation failed");

  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
