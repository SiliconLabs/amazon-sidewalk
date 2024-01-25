/***************************************************************************//**
 * @file
 * @brief common.c
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <sid_error.h>
#include <sid_pal_common_ifc.h>
#include <delay.h>

#if defined(SV_ENABLED)
extern void silabs_crypto_enable_sv(void);
#endif // SV_ENABLED

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
sid_error_t sid_pal_common_init(const platform_specific_init_parameters_t *platform_init_parameters)
{
  if (!platform_init_parameters
#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
      || !platform_init_parameters->radio_cfg
#endif
      ) {
    return SID_ERROR_INCOMPATIBLE_PARAMS;
  }

  // Initialise platform-specific & hardware-dependent blocks
  silabs_delay_init();

#if defined(SV_ENABLED)
  silabs_crypto_enable_sv();
#endif // SV_ENABLED

#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
#if defined(SL_RADIO_EXTERNAL)
  set_radio_sx126x_device_config(platform_init_parameters->radio_cfg);
#elif defined(SL_RADIO_NATIVE)
  set_radio_efr32xgxx_device_config(platform_init_parameters->radio_cfg);
#endif
#endif

  return SID_ERROR_NONE;
}

sid_error_t sid_pal_common_deinit(void)
{
#if (defined(SL_FSK_SUPPORTED) || defined(SL_CSS_SUPPORTED))
  sid_pal_radio_deinit();
#endif

  return SID_ERROR_NONE;
}
