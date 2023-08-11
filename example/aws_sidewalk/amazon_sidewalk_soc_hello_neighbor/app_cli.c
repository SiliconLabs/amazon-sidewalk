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

void cli_send(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_connect_and_send();
}

void cli_reset(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_factory_reset();
}

void cli_get_connection_status(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_get_connection_status();
}

void cli_get_time(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_get_time();
}

void cli_get_mtu(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_trigger_get_mtu();
}
