/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_nvm3_migrator.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SIDEWALK_NVM3_MIGRATOR_H
#define SL_SIDEWALK_NVM3_MIGRATOR_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief API function of the NVM3 migrator component, transforming sidewalk
 *        NVM3 from the pre-1.16 structure to a new structure.
 *
 * Capabilities:
 *   - Handles MFG, KV storage and generic NVM3 data.
 *   - Adds a version number representing the new NVM3 structure.
 *   - Provides minimal runtime and memory overhead by quickly returning if the
 *     version is up to date and by using mostly dynamic memory allocation.
 *   - Is resistant to power off: able to continue migration after power off
 *     without losing data.
 *   - Uses API assertions. (There is no point in returning an error code to the
 *     application in case of errors preventing migration.)
 *   - Is aware of the device backup functionality, tolerates empty NVM3 if the
 *     backup component is present.
 *
 * @note Contains direct NVM3 API calls.
 *****************************************************************************/
void sl_sidewalk_nvm3_migrator_run(void);

#ifdef __cplusplus
}
#endif

#endif // SL_SIDEWALK_NVM3_MIGRATOR_H
