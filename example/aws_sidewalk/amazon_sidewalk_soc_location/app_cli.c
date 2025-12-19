/***************************************************************************//**
 * @file
 * @brief app_cli.c
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
#include "sl_cli.h"
#include "app_process.h"
#include <sid_location.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
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
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void cli_link_switch(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_link_switch();
}

void cli_ble_connect(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_connection_request();
}

void cli_send(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_send_counter_update();
}

void cli_reset(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_factory_reset();
}

void sl_sidewalk_location_cli_init(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_location_init();
}

void sl_sidewalk_location_cli_deinit(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_location_deinit();
}

void sl_sidewalk_location_cli_send(sl_cli_command_arg_t *arguments)
{
  enum sid_location_effort_mode effort = sl_cli_get_argument_uint32(arguments, 0);
  app_trigger_location_send(effort);
}

#if defined(SL_LOCATION_FULL)
void sl_sidewalk_location_cli_send_buf(sl_cli_command_arg_t *arguments)
{
  enum sid_location_effort_mode effort = sl_cli_get_argument_uint32(arguments, 0);
  app_trigger_location_send_buf(effort);
}

void sl_sidewalk_location_cli_scan(sl_cli_command_arg_t *arguments)
{
  enum sid_location_effort_mode effort = sl_cli_get_argument_uint32(arguments, 0);
  app_trigger_location_scan(effort);
}
#endif

void sl_sidewalk_location_cli_alm_start(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_location_alm_start();
}
