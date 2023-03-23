/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_cli_core.h
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

#ifndef SL_SIDEWALK_CLI_CORE_H
#define SL_SIDEWALK_CLI_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_status.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Unlock th CLI mutex and return.
 *****************************************************************************/
#define sl_app_sidewalk_release_cli_mutex_and_return() \
  do {                                                 \
    sl_app_sidewalk_cli_mutex_unlock();                \
    return;                                            \
  } while (0)

/**************************************************************************//**
 * @brief Unlock the CLI mutex and return with a value.
 *****************************************************************************/
#define sl_app_sidewalk_release_cli_mutex_and_return_val(__val) \
  do {                                                          \
    sl_app_sidewalk_cli_mutex_unlock();                         \
    return (__val);                                             \
  } while (0)

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief Initialize the CLI components mutex.
 * @details This function initializes the CLI mutex.
 *****************************************************************************/
void sl_app_sidewalk_cli_init(void);

/**************************************************************************//**
 * @brief Lock by mutex.
 *****************************************************************************/
void sl_app_sidewalk_cli_mutex_lock(void);

/**************************************************************************//**
 * @brief Unlock by mutex.
 *****************************************************************************/
void sl_app_sidewalk_cli_mutex_unlock(void);

#ifdef __cplusplus
}
#endif

#endif  // SL_SIDEWALK_CLI_CORE_H