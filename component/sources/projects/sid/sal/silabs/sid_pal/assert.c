/***************************************************************************//**
 * @file
 * @brief assert.c
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
#include <sid_pal_assert_ifc.h>
#include <sid_pal_log_ifc.h>
#include <app_assert.h>
#include "cmsis_compiler.h"
#include <stdint.h>

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

/*******************************************************************************
 * Assert callback function, can be overwritten by user later
 * @param line_num Where the assert happened
 * @param file_name Which file was asserted
 ******************************************************************************/
__WEAK void sl_assert_app_callback(uint16_t line_num,
                                   const char * file_name);

/*******************************************************************************
 * Assert function to stop the application at a certain point
 * @param line_num Where the assert happened
 * @param file_name Which file was asserted
 ******************************************************************************/
void sid_pal_assert(int line,
                    const char * file)
{
  sl_assert_app_callback(line, file);

  while (1) {
  }
}

/*******************************************************************************
 * Assert callback function, can be overwritten by user later
 * @param line_num Where the assert happened
 * @param file_name Which file was asserted
 ******************************************************************************/
__WEAK void sl_assert_app_callback(uint16_t line_num,
                                   const char * file_name)
{
  SID_PAL_LOG_ERROR("Received a fault! %s @ %d", file_name, line_num);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
