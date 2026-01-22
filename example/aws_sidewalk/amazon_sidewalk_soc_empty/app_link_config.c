/***************************************************************************//**
 * @file
 * @brief app_link_config.c
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
#include "sid_api.h"

#if defined(SL_BLE_SUPPORTED)
#include "app_ble_config.h"
#endif // SL_BLE_SUPPORTED

#if defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED)
#include "app_subghz_config.h"
#endif // SL_FSK_SUPPORTED || SL_CSS_SUPPORTED

#include "app_link_config.h"

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void app_link_config(struct sid_config *config)
{
#if defined(SL_BLE_SUPPORTED)
  config->link_config = app_get_ble_config();
#else
  config->link_config = NULL;
#endif // SL_BLE_SUPPORTED

#if defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED)
  config->sub_ghz_link_config = app_get_sub_ghz_config();
#else
  config->sub_ghz_link_config = NULL;
#endif // SL_FSK_SUPPORTED || SL_CSS_SUPPORTED
}
