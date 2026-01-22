/***************************************************************************//**
 * @file
 * @brief sid_pal_timer_types.h
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

#ifndef SID_PAL_TIMER_TYPES_H
#define SID_PAL_TIMER_TYPES_H

/**
 * \addtogroup sid_pal
 * @{
 */
/**
 * \addtogroup sid_pal_tmr_types
 * @{
 */

/* ----------------------------------------------------------------------------- */
/*                                   Includes */
/* ----------------------------------------------------------------------------- */
#include <sid_time_types.h>
#include <sl_sleeptimer.h>

/* ----------------------------------------------------------------------------- */
/*                              Macros and Typedefs */
/* ----------------------------------------------------------------------------- */
/**************************************************************************//**
 * @addtogroup sid_pal_tmr_types_types Type definitions
 * @ingroup sid_pal_tmr_types
 * @{
 *****************************************************************************/
/**
 * @brief Typedef for the timer implementation structure.
 *
 * This typedef defines a type alias for the timer implementation structure.
 */
typedef struct sid_pal_timer_impl_t sid_pal_timer_t;

/**
 * @brief Timer callback type
 *
 * @note The callback is allowed to execute absolute minimum amount of work and return as soon as possible
 * @note Implementer of the callback should consider the callback is executed from ISR context
 */
typedef void (* sid_pal_timer_cb_t)(void * arg,
                                    sid_pal_timer_t * originator);

/**
 * @brief Timer storage type
 *
 * @note This is the implementor defined storage type for timers.
 */
struct sid_pal_timer_impl_t{
  struct sid_timespec alarm;                      /*!< The alarm time for the timer. */
  struct sid_timespec period;                     /*!< The period of the timer. */
  sl_sleeptimer_timer_handle_t sleeptimer_handle; /*!< Handle for the sleep timer. */
  sid_pal_timer_cb_t callback;                    /*!< Callback function to be called when the timer expires. */
  bool is_periodic;                               /*!< Indicates if the timer is periodic. */
  bool has_started;                               /*!< Indicates if the timer has started. */
  uint32_t period_in_ms;                          /*!< The period of the timer in milliseconds. */
  void * callback_arg;                            /*!< Argument to be passed to the callback function. */
};

/** @} (end sid_pal_tmr_types_types) */

/* ----------------------------------------------------------------------------- */
/*                                Global Variables */
/* ----------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------- */
/*                          Public Function Declarations */
/* ----------------------------------------------------------------------------- */

#endif /* SID_PAL_TIMER_TYPES_H */

/** @} */ // end of sid_pal group
/** @} */ // end of sid_pal_tmr_types group