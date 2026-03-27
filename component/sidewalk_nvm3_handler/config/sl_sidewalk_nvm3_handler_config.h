/***************************************************************************//**
 * @file
 * @brief Sidewalk nvm3 handler component configuration
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

#ifndef SL_SIDEWALK_NVM3_HANDLER_CONFIG_H
#define SL_SIDEWALK_NVM3_HANDLER_CONFIG_H

#include "nvm3_default_config.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Sidewalk NVM3 handler configuration

// <o SL_SIDEWALK_NVM3_CACHE_SIZE> NVM3 Cache Size
// <i> Number of NVM3 objects to cache. To reduce access times this number
// <i> should be equal to or higher than the number of NVM3 objects in the
// <i> NVM3 instance.
// <i> Default: NVM3_DEFAULT_CACHE_SIZE
#ifndef SL_SIDEWALK_NVM3_CACHE_SIZE
#define SL_SIDEWALK_NVM3_CACHE_SIZE                 NVM3_DEFAULT_CACHE_SIZE
#else
#error "NVM3_DEFAULT_CACHE_SIZE should not be overwritten"
#endif

// <o SL_SIDEWALK_NVM3_MAX_OBJECT_SIZE> NVM3 Instance Max Object Size
// <i> Max NVM3 object size that can be stored.
// <i> Default: NVM3_DEFAULT_MAX_OBJECT_SIZE
#ifndef SL_SIDEWALK_NVM3_MAX_OBJECT_SIZE
#define SL_SIDEWALK_NVM3_MAX_OBJECT_SIZE            NVM3_DEFAULT_MAX_OBJECT_SIZE
#else
#error "SL_SIDEWALK_NVM3_MAX_OBJECT_SIZE should not be overwritten"
#endif

// <o SL_SIDEWALK_NVM3_DEFAULT_SIZE> NVM3 Default Size
// <i> Size of the NVM3 storage region in flash. This size should be aligned with
// <i> the flash page size of the device.
// <i> Default: NVM3_DEFAULT_NVM_SIZE
#ifndef SL_SIDEWALK_NVM3_DEFAULT_SIZE
#define SL_SIDEWALK_NVM3_DEFAULT_SIZE               NVM3_DEFAULT_NVM_SIZE
#else
#error "SL_SIDEWALK_NVM3_DEFAULT_SIZE should not be overwritten"
#endif

// <o SL_SIDEWALK_NVM3_DEFAULT_REPACK_HEADROOM> NVM3 Default Repack Headroom
// <i> Headroom determining how many bytes below the forced repack limit the user
// <i> repack limit should be placed. The default is 0, which means the user and
// <i> forced repack limits are equal.
// <i> Default: NVM3_DEFAULT_REPACK_HEADROOM
#ifndef SL_SIDEWALK_NVM3_DEFAULT_REPACK_HEADROOM
#define SL_SIDEWALK_NVM3_DEFAULT_REPACK_HEADROOM    NVM3_DEFAULT_REPACK_HEADROOM
#else
#error "SL_SIDEWALK_NVM3_DEFAULT_REPACK_HEADROOM should not be overwritten"
#endif

// </h>

// <<< end of configuration section >>>

#endif // SL_SIDEWALK_NVM3_HANDLER_CONFIG_H
