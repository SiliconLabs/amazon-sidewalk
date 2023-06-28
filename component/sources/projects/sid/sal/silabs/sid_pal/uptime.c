/***************************************************************************//**
 * @file
 * @brief uptime.c
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
#include <string.h>
#include <sid_pal_uptime_ifc.h>
#include <sl_sleeptimer.h>
#include <sid_time_ops.h>

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

/**
 * Get the current time of specified clock source
 *
 * @param[out]  time            current time
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * NOTE: drift may be NULL. In this case time should be set and drift ignored. Success should be returned.
 *
 */
sid_error_t sid_pal_uptime_now(struct sid_timespec * time)
{
  uint64_t ticks = sl_sleeptimer_get_tick_count64();
  uint32_t ticks_per_sec = sl_sleeptimer_get_timer_frequency();
  uint64_t secs = ticks / ticks_per_sec;

  uint64_t residual_ticks = ticks % ticks_per_sec;
  time->tv_sec = secs;
  time->tv_nsec = (residual_ticks * SID_TIME_NSEC_PER_SEC) / ticks_per_sec;

  return SID_ERROR_NONE;
}

/*******************************************************************************
 * Set crystal offset for RTC compensation
 *
 * @param[in]   ppm          offset in PPM
 *
 ******************************************************************************/
void sid_pal_uptime_set_xtal_ppm(int16_t ppm)
{
  (void)(ppm);
}

/*******************************************************************************
 * Get current crystal offset
 *
 * @retval offset in PPM
 ******************************************************************************/
int16_t sid_pal_uptime_get_xtal_ppm(void)
{
  return 0;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
