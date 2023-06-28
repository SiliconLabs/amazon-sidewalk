/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_common_config.h
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

#ifndef SL_SIDEWALK_COMMON_CONFIG_H
#define SL_SIDEWALK_COMMON_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>
#define SL_SIDEWALK_LINK_BLE    1
#define SL_SIDEWALK_LINK_FSK    2
#define SL_SIDEWALK_LINK_CSS    3


// <h> Sidewalk Common Configuration
// <o SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE> Default link type
// <SL_SIDEWALK_LINK_BLE=> BLE
// <SL_SIDEWALK_LINK_FSK=> FSK
// <SL_SIDEWALK_LINK_CSS=> CSS
// <i> Default: SL_SIDEWALK_LINK_BLE
#define SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE  SL_SIDEWALK_LINK_BLE
// </h>

// <<< end of configuration section >>>

#if SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE == SL_SIDEWALK_LINK_CSS

#if defined(SL_FSK_SUPPORTED)
#define SL_SIDEWALK_COMMON_REGISTRATION_LINK SL_SIDEWALK_LINK_FSK
#elif defined(SL_BLE_SUPPORTED)
#define SL_SIDEWALK_COMMON_REGISTRATION_LINK SL_SIDEWALK_LINK_BLE
#else
#warning Please note registration is not supported on CSS link!
#endif

#else
#define SL_SIDEWALK_COMMON_REGISTRATION_LINK SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE
#endif

// Sanity check that the default link is supported on current platform
#if (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE == SL_SIDEWALK_LINK_BLE) && (!defined(SL_BLE_SUPPORTED))
  #error "SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE:BLE link is not supported on this platform!"
#elif (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE == SL_SIDEWALK_LINK_FSK) && (!defined(SL_FSK_SUPPORTED))
  #error "SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE:FSK link is not supported on this platform!"
#elif (SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE == SL_SIDEWALK_LINK_CSS) && (!defined(SL_CSS_SUPPORTED))
  #error "SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE:CSS link is not supported on this platform!"
#endif

#endif // SL_SIDEWALK_COMMON_CONFIG_H