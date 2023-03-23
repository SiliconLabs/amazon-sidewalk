/***************************************************************************//**
 * @file sid_pal_timer_types.h
 * @brief sid_pal_timer_types.h
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SID_PAL_TIMER_TYPES_H
#define SID_PAL_TIMER_TYPES_H

/* ----------------------------------------------------------------------------- */
/*                                   Includes */
/* ----------------------------------------------------------------------------- */
#include <sid_time_types.h>
#include <rail_types.h>

/* ----------------------------------------------------------------------------- */
/*                              Macros and Typedefs */
/* ----------------------------------------------------------------------------- */
typedef struct sid_pal_timer_impl_t sid_pal_timer_t;

/*******************************************************************************
 * @brief Timer callback type
 *
 * @note The callback is allowed to execute absolute minimum amount of work and return as soon as possible
 * @note Implementer of the callback should consider the callback is executed from ISR context
 ******************************************************************************/
typedef void (* sid_pal_timer_cb_t)(void * arg,
                                    sid_pal_timer_t * originator);

/*******************************************************************************
 * @brief Timer storage type
 *
 * @note This is the implementor defined storage type for timers.
 ******************************************************************************/
struct sid_pal_timer_impl_t
{
    struct sid_timespec alarm;
    struct sid_timespec period;
    RAIL_MultiTimer_t rail_timer_handle;
    sid_pal_timer_cb_t callback;
    bool is_periodic;
    uint32_t event_cnt;
    uint32_t start_offset_in_ticks;
    uint32_t period_in_ticks;
    uint64_t next_period;
    void * callback_arg;
};

/* ----------------------------------------------------------------------------- */
/*                                Global Variables */
/* ----------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------- */
/*                          Public Function Declarations */
/* ----------------------------------------------------------------------------- */

#endif /* SID_PAL_TIMER_TYPES_H */
