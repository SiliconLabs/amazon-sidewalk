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
void cli_send(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_dmp_soc_light_update_counter_ctx_t ctx = {
    .hdl.operation = SL_SID_APP_MSG_OP_NTFY
  };
  app_trigger_update_counter(&ctx);
#endif
}

void cli_reset(sl_cli_command_arg_t *arguments)
{
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_dev_mgmt_rst_dev_ctx_t ctx = {
    .param_send.in_millisecs = sl_cli_get_argument_uint32(arguments, 0),
    .param_send.reset_type = sl_cli_get_argument_uint8(arguments, 1),
    .hdl.operation = SL_SID_APP_MSG_OP_NTFY
  };
  app_trigger_device_reset(&ctx);
#else
  (void)arguments;
#endif
}

void cli_get_time(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_sid_time_ctx_t ctx = {
    .hdl.operation = SL_SID_APP_MSG_OP_NTFY
  };
  app_trigger_time(&ctx);
#endif
}

void cli_get_mtu(sl_cli_command_arg_t *arguments)
{
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_sid_mtu_ctx_t ctx = {
    .param_rcv.link_type = sl_cli_get_argument_uint8(arguments, 0),
    .hdl.operation = SL_SID_APP_MSG_OP_NTFY
  };
  app_trigger_mtu(&ctx);
#else
  (void)arguments;
#endif
}

void cli_toggle_led(sl_cli_command_arg_t *arguments)
{
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_dev_mgmt_toggle_led_ctx_t ctx = {
    .param_send.led = sl_cli_get_argument_uint8(arguments, 0),
    .hdl.operation = SL_SID_APP_MSG_OP_NTFY
  };
  app_trigger_toggle_led(&ctx);
#else
  (void)arguments;
#endif
}

void cli_ble_start_stop(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
#if defined(SL_SID_APP_MSG_PRESENT)
  sl_sid_app_msg_dmp_soc_light_ble_start_stop_ctx_t ctx = {
    .hdl.operation = SL_SID_APP_MSG_OP_NTFY
  };
  app_trigger_ble_start_stop(&ctx);
#endif
}
