/***************************************************************************//**
 * @file
 * @brief critical_region.c
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
#include <sid_pal_critical_region_ifc.h>

#include <FreeRTOS.h>
#include <task.h>

#include <stdatomic.h>
#include <em_core.h>

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static atomic_int count = ATOMIC_VAR_INIT(0);

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sid_pal_enter_critical_region(void)
{
  CORE_ATOMIC_IRQ_DISABLE();
  const unsigned int prev_val = atomic_fetch_add(&count, 1);
  SID_PAL_ASSERT(prev_val <= 8);    // Some maximum amount of re-entry
}

void sid_pal_exit_critical_region(void)
{
  const unsigned int prev_val = atomic_fetch_sub(&count, 1);
  SID_PAL_ASSERT(prev_val > 0);
  if (prev_val == 1) {
    CORE_ATOMIC_IRQ_ENABLE();
  }
}
