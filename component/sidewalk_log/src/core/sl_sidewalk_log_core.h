/***************************************************************************//**
 * @file sl_sidewalk_log_core.h
 * @brief sidewalk log core
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SIDEWALK_LOG_CORE_H
#define SL_SIDEWALK_LOG_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "sl_iostream.h"
#include "sl_iostream_handles.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

typedef enum {
  SL_SID_LOG_CORE_SEVERITY_ERROR = 0,
  SL_SID_LOG_CORE_SEVERITY_WARNING = 1,
  SL_SID_LOG_CORE_SEVERITY_INFO = 2,
  SL_SID_LOG_CORE_SEVERITY_DEBUG = 3
} sl_sid_log_core_log_severity_t;

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Print log on the chosen iostream
 *
 *  @param[in] severity Log severity
 *  @param[in] fmt Formatted string
 *  @param[in] args Arguments to the formatted string
 ******************************************************************************/
void sl_sid_log_core_print(sl_sid_log_core_log_severity_t severity, const char *fmt, va_list args);

/*******************************************************************************
 * Set the iostream to be used to print the logs
 *
 *  @param[in] iostream iostream to be set
 ******************************************************************************/
void sl_sid_log_core_set_iostream(sl_iostream_t *iostream);

#if defined(SID_PAL_LOG_RTT_ENABLED)
/*******************************************************************************
 * Set the RTT terminal ID to be used to print the logs
 *
 *  @param[in] terminal_id Terminal ID
 ******************************************************************************/
void sl_sid_log_core_set_rtt_terminal_id(uint32_t terminal_id);
#endif

#ifdef __cplusplus
}
#endif

#endif  // SL_SIDEWALK_LOG_CORE_H
