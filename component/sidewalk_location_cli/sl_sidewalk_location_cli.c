/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_location_cli.c
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 * Your use of this software is governed by the terms of
 * Silicon Labs Master Software License Agreement (MSLA)available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * This software contains Third Party Software licensed by Silicon Labs from
 * Amazon.com Services LLC and its affiliates and is governed by the sections
 * of the MSLA applicable to Third Party Software and the additional terms set
 * forth in amazon_sidewalk_license.txt.
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

#include "sl_cli.h"
#include "sl_sidewalk_log_app.h"
#include "sl_sidewalk_location_cli.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Define the WEAK macro for GCC compatible compilers
#ifndef WEAK
#define WEAK __attribute__((weak))
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

struct sid_location_config sl_sidewalk_location_cli_config = {
#if defined(SL_LOCATION_FULL)
  .sid_location_type_mask = SID_LOCATION_METHOD_ALL,
#else
  .sid_location_type_mask = SID_LOCATION_METHOD_BLE_GATEWAY,
#endif
  .manage_effort = true,
#if defined(SL_LOCATION_FULL)
  .max_effort = SID_LOCATION_EFFORT_L4,
#else
  .max_effort = SID_LOCATION_EFFORT_L1,
#endif
  .callbacks = {
    .on_update = sl_sidewalk_location_cli_callback,
  },
  .stepdowns = {
    .l2_to_l1 = 0,
    .l3_to_l2 = 0,
    .l4_to_l3 = 0,
  },
  .fragmentation = {
    .timeout_ms = 0,
    .max_retries = 1,
  }
};

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Provide the location result to the application
 *
 * This function provides the location result provided in the location callback
 * to the application for further processing.
 *
 * @param[in] payload The payload of the location result
 * @param[in] size The size of the location result
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_handle_result(const uint8_t* const payload, size_t size)
{
  (void)payload;
  (void)size;
  SL_SID_LOG_APP_INFO("location cli: handle_result not implemented");
}

/**************************************************************************//**
 * @brief Callback function for the location API
 *
 * @param[in] result The result of the location update
 * @param[in] context The context of the location update
 *****************************************************************************/
void sl_sidewalk_location_cli_callback(const struct sid_location_result *const result, void *context)
{
  (void)context;

  SL_SID_LOG_APP_INFO("location status: %d", result->status);
  SL_SID_LOG_APP_INFO("location send result: %d", result->err);
  SL_SID_LOG_APP_INFO("location effort mode: %d", result->mode);
  SL_SID_LOG_APP_INFO("location link type: %d", result->link);

  if ((result->mode == SID_LOCATION_EFFORT_L3) ||
      (result->mode == SID_LOCATION_EFFORT_L4)) {
    // For L3 and L4 print the locations data, regardless
    // of the location API result (success or error)
    // of the request (send or scan)
    SL_SID_LOG_APP_INFO("location payload:");
    SL_SID_LOG_APP_HEXDUMP_INFO(result->payload, result->size);

    // In case of successfult scan provide the ruslt to the application
    if ((result->err == SID_ERROR_NONE) &&
        (result->status == SID_LOCATION_SCAN_DONE) &&
        (result->size > 0)) {
      sl_sidewalk_location_cli_handle_result(result->payload, result->size);
    }
  }
}

/**************************************************************************//**
 * @brief CLI command to initialize the location service
 *
 * This function initializes the Sidewalk location service with the configured
 * parameters including location methods, effort levels, and callbacks.
 *
 * @param[in] arguments CLI command arguments (unused)
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_init(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  SL_SID_LOG_APP_INFO("location cli: init not implemented");
}

/**************************************************************************//**
 * @brief CLI command to deinitialize the location service
 *
 * @param[in] arguments CLI command arguments (unused)
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_deinit(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  SL_SID_LOG_APP_INFO("location cli: deinit not implemented");
}

/**************************************************************************//**
 * @brief CLI command to scan for location data and send it
 *
 * This function performs a location scan using the specified effort mode and
 * sends the collected location data over the Sidewalk network. The effort mode
 * determines which location methods are used (BLE, GNSS, WiFi) and the scan
 * duration.
 *
 * @param[in] arguments CLI command arguments containing the effort mode (L1-L4)
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_send(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  SL_SID_LOG_APP_INFO("location cli: send not implemented");
}

/**************************************************************************//**
 * @brief CLI command to send a pre-filled buffer as location data
 *
 * This function sends a test buffer filled with 0xFF bytes as location data
 * without performing any actual location scanning. This is useful for testing
 * the location transmission functionality. Only supports effort modes L3 and
 * L4.
 *
 * @param[in] arguments CLI command arguments containing the effort mode (L3-L4)
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_send_buf(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  SL_SID_LOG_APP_INFO("location cli: send_buf not implemented");
}

/**************************************************************************//**
 * @brief CLI command to perform location scanning without sending data
 *
 * This function performs location scanning using the specified effort mode
 * but does not transmit the collected data. The scanned data will be available
 * in the callback for inspection. Only supports effort modes L3 and L4.
 *
 * @param[in] arguments CLI command arguments containing the effort mode (L3-L4)
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_scan(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  SL_SID_LOG_APP_INFO("location cli: scan not implemented");
}

/**************************************************************************//**
 * @brief CLI command to start GNSS almanac demodulation
 *
 * This function starts the GNSS almanac demodulation process, which is used
 * to receive and decode almanac data from GNSS satellites. This feature is
 * only available when GNSS support is enabled in the SDK configuration.
 *
 * @param[in] arguments CLI command arguments (unused)
 *****************************************************************************/
WEAK void sl_sidewalk_location_cli_alm_start(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  SL_SID_LOG_APP_INFO("location cli: alm_start not implemented");
}
