/***************************************************************************//**
 * @file
 * @brief timer.c
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
#include <sid_pal_timer_ifc.h>
#include <sid_pal_uptime_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_pal_assert_ifc.h>
#include <sid_time_ops.h>
#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// RAIL does not allow to set timers for more than ~3221 seconds
#define RAIL_MAX_ALLOWED_TIMEOUT_VAL_SEC ((((~((uint32_t)0) / 4U) * 3U) / SID_TIME_USEC_PER_SEC) - 1)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Sleeptimer callback to call the proper timer objects callback with argument
 ******************************************************************************/
static void sleeptimer_callback(sl_sleeptimer_timer_handle_t * handle,
                                void * data);

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
 * Initialize a timer object
 *
 * @param[in]   timer               Timer object to initialize
 * @param[in]   event_callback      Pointer to the callback function the timer event will be delivered to
 * @param[in]   event_callback_arg  Argument to be provided to the @p event_callback during call
 *
 * @retval SID_ERROR_NONE in case of success
 ******************************************************************************/
sid_error_t sid_pal_timer_init(sid_pal_timer_t * timer,
                               sid_pal_timer_cb_t event_callback,
                               void * event_callback_arg)
{
  if (!timer || !event_callback) {
    return SID_ERROR_INVALID_ARGS;
  }
  timer->callback = event_callback;
  timer->callback_arg = event_callback_arg;
  timer->alarm = SID_TIME_INFINITY;
  timer->period = SID_TIME_INFINITY;
  return SID_ERROR_NONE;
}

/*******************************************************************************
 * De-initialize a timer object
 *
 * @param[in]   timer               Timer object to de-initialize
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function fully de-initializes the @p timer object. If it is armed, it will be canceled and then de-initialized.
 ******************************************************************************/
sid_error_t sid_pal_timer_deinit(sid_pal_timer_t * timer)
{
  if (!timer) {
    return SID_ERROR_INVALID_ARGS;
  }
  bool running = false;
  sl_sleeptimer_is_timer_running(&(timer->sleeptimer_handle), &running);

  if (running) {
    sl_sleeptimer_stop_timer(&(timer->sleeptimer_handle));
  }
  timer->callback = NULL;
  timer->callback_arg = NULL;
  timer->alarm = SID_TIME_ZERO;
  timer->period = SID_TIME_ZERO;
  return SID_ERROR_NONE;
}

/*******************************************************************************
 * Arm a timer object
 *
 * @param[in]   timer               Timer object to arm
 * @param[in]   type                Priority class specifier for the timer to be armed
 * @param[in]   when                Pointer to struct sid_timespec identifying the time for the first event generation
 * @param[in]   period              Pointer to struct sid_timespec identifying the period between event generation
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function will initialize the @p timer object for first shot at time provided in @p when (required). If
 * the @p period is not NULL and is not TIMESPEC_INFINITY, the @p timer object will be armed to repeat events
 * generation periodically with the period according to the time provided in @p period.
 ******************************************************************************/
sid_error_t sid_pal_timer_arm(sid_pal_timer_t * timer,
                              sid_pal_timer_prio_class_t type,
                              const struct sid_timespec * when,
                              const struct sid_timespec * period)
{
  if (!timer || !when) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (sid_pal_timer_is_armed(timer)) {
    return SID_ERROR_INVALID_ARGS;
  }
  timer->alarm =  *when;

  if (period != NULL) {
    timer->period = *period;
    timer->is_periodic = true;
    timer->has_started = false;
  }
  uint8_t priority = 0;

  switch (type) {
    case SID_PAL_TIMER_PRIO_CLASS_PRECISE:
      priority = 0;
      break;

    case SID_PAL_TIMER_PRIO_CLASS_LOWPOWER:
      priority = 10;
      break;

    default:
      break;
  }

  volatile uint64_t timeout_tick = 0;
  volatile uint64_t timeout_us = 0;
  struct sid_timespec up_time;
  struct sid_timespec alarm_cp = timer->alarm;

  up_time.tv_nsec = 0;
  up_time.tv_sec = 0;

  if (period != NULL) {
    timer->period_in_ms = (timer->period.tv_sec * 1000) + (timer->period.tv_nsec / 1000000);
  }

  sid_pal_uptime_now(&up_time);

  // from here to sl_sleeptimer_start_timer it takes around 6 us, so
  // we add this offset value to uptime value to avoid arming the timer with
  // a value in the past
  up_time.tv_nsec += 6000;

  if (sid_time_gt(&alarm_cp, &up_time)) {
    // arm a one-shot timer first to handle the first timer arming which is supposed to be a bit shorter
    // than the period due to code execution time between the caller and the actual timer start (a few lines below)
    sid_time_sub(&alarm_cp, &up_time);
    timeout_us = (alarm_cp.tv_sec * 1000000) + (alarm_cp.tv_nsec / 1000);       // convert from sid_timespec to us
    timeout_tick = ((timeout_us * 32768) + 999999) / 1000000;                   // convert from us to tick and round up
  }
  int status = sl_sleeptimer_start_timer(&(timer->sleeptimer_handle), timeout_tick, sleeptimer_callback, timer, priority, 0);
  if (status != SID_ERROR_NONE) {
    SID_PAL_LOG_ERROR("pal: arm timer failed");
  }
  return SID_ERROR_NONE;
}

/*******************************************************************************
 * Disarm a timer object
 *
 * @param[in]   timer               Timer object to disarm
 *
 * @retval SID_ERROR_NONE in case of success
 *
 * Function will disarm the @p timer object. If it is not armed, function does no operation.
 ******************************************************************************/
sid_error_t sid_pal_timer_cancel(sid_pal_timer_t * timer)
{
  if (!timer) {
    return SID_ERROR_INVALID_ARGS;
  }

  if (sid_pal_timer_is_armed(timer)) {
    sl_sleeptimer_stop_timer(&(timer->sleeptimer_handle));
  }
  return SID_ERROR_NONE;
}

/*******************************************************************************
 * Check a timer object is valid and armed
 *
 * @param[in]   timer               Timer object to check
 *
 * @retval true in case of @p timer object is armed
 * @retval false in case of @p timer object is disarmed, deinitialized or invalid
 *
 ******************************************************************************/
bool sid_pal_timer_is_armed(const sid_pal_timer_t * timer)
{
  bool running = false;

  sl_sleeptimer_is_timer_running(&(((sid_pal_timer_t *)timer)->sleeptimer_handle), &running);
  return running;
}

/*******************************************************************************
 * Init the timer facility. This function must be called before before sid_pal_timer_init().
 *
 * OPTIONAL This function is typically used to init HW or SW resources needed for the timer.
 * If none are needed by the timer implementation then this function is unnecessary.
 *
 * @retval SID_ERROR_NONE in case of success
 ******************************************************************************/
sid_error_t sid_pal_timer_facility_init(void * arg)
{
  (void)(arg);
  return SID_ERROR_NONE;
}

/*******************************************************************************
 * HW event callback
 *
 * OPTIONAL If sid_timer is implemented as a SW timer, this is the callback that can be
 * registered with the HW resource to provide noritification of HW timer expiry.
 ******************************************************************************/
void sid_pal_timer_event_callback(void * arg,
                                  const struct sid_timespec * now)
{
  (void)(arg);
  (void)(now);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Sleeptimer callback to call the proper timer objects callback with argument
 * @param handle Which sleeptimer called this callback
 * @param data Data which was given in when timer was started
 ******************************************************************************/
static void sleeptimer_callback(sl_sleeptimer_timer_handle_t * handle,
                                void * data)
{
  (void)(handle);
  sid_pal_timer_t * timer = (sid_pal_timer_t *)data;
  if (timer->is_periodic && !timer->has_started) {
    timer->has_started = true;
    sl_status_t ret = sl_sleeptimer_start_periodic_timer_ms(&(timer->sleeptimer_handle), timer->period_in_ms, sleeptimer_callback, timer, 0, 0);
    SID_PAL_ASSERT(ret == SL_STATUS_OK);
  }
  timer->callback(timer->callback_arg, (sid_pal_timer_t *)timer);
}
