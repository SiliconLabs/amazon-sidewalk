/***************************************************************************//**
 * @file
 * @brief main.c
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
#include "app_init.h"
#include "sl_bt_api.h"
#include <stdbool.h>
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
int main(void)
{
  // Initialize the application
  app_init();

  while (1) {
  }
}

/*
   THIS IS FOR MAXIMIZING TX POWER FOR ADVERTISING.
 */
void sl_bt_on_event(sl_bt_msg_t* evt)
{
  static int16_t support_min;
  static int16_t support_max;
  static int16_t set_min;
  static int16_t set_max;
  static int16_t rf_path_gain;

  switch ( SL_BT_MSG_ID(evt->header) ) {
    case sl_bt_evt_system_boot_id:
      sl_bt_system_get_tx_power_setting(&support_min, &support_max, &set_min, &set_max, &rf_path_gain);
      sl_bt_system_set_tx_power(support_min, support_max, &set_min, &set_max);
      break;
  }
}
/*
   END OF TX POWER
 */
