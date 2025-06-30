/***************************************************************************//**
 * @file sl_sidewalk_log_core.c
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_component_catalog.h"
#if !defined(SL_CATALOG_SIDEWALK_PDP_PRESENT)
#include "sl_sidewalk_common_config.h"
#endif
#if defined(SL_CATALOG_SIDEWALK_LOG_APP_RTT_PRESENT)
#include "sl_sidewalk_log_app_rtt_config.h"
#else
#include "sl_sidewalk_log_core.h"
#endif
#include "app_log.h"
#include "sid_clock_ifc.h"
#if defined(SID_PAL_LOG_RTT_ENABLED)
#include "SEGGER_RTT.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define LOG_MAX_BUFFER_CHAR (256)

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static uint32_t get_time_now(void);
static void sli_sid_log_print_fmt(sl_sid_log_core_log_severity_t severity, const char *buffer);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static sl_iostream_t *default_iostream = NULL;
#if defined(SID_PAL_LOG_RTT_ENABLED)
static uint32_t default_rtt_terminal_id = 0;
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sl_sid_log_core_set_iostream(sl_iostream_t *iostream)
{
  default_iostream = iostream;
}

#if defined(SID_PAL_LOG_RTT_ENABLED)
void sl_sid_log_core_set_rtt_terminal_id(uint32_t terminal_id)
{
  default_rtt_terminal_id = terminal_id;
}
#endif

static void sli_sid_log_print_fmt(sl_sid_log_core_log_severity_t severity, const char *buffer)
{
  switch (severity) {
    case SL_SID_LOG_CORE_SEVERITY_ERROR:
      app_log_error(buffer);
      break;

    case SL_SID_LOG_CORE_SEVERITY_WARNING:
      app_log_warning(buffer);
      break;

    case SL_SID_LOG_CORE_SEVERITY_INFO:
      app_log_info(buffer);
      break;

    case SL_SID_LOG_CORE_SEVERITY_DEBUG:
      app_log_debug(buffer);
      break;

    default:
      break;
  }
}

void sl_sid_log_core_print(sl_sid_log_core_log_severity_t severity, const char *fmt, va_list args)
{
  char buffer[LOG_MAX_BUFFER_CHAR];

  app_log_iostream_set(default_iostream);
#if defined(SID_PAL_LOG_RTT_ENABLED)
  SEGGER_RTT_SetTerminal((uint8_t)default_rtt_terminal_id);
#endif

  vsnprintf(buffer, LOG_MAX_BUFFER_CHAR, fmt, args);

  if (app_log_iostream_get() == sl_iostream_get_handle("rtt")) {
#if defined(SID_PAL_LOG_RTT_ENABLED)
    SEGGER_RTT_LOCK();
#endif
    sli_sid_log_print_fmt(severity, (const char *)buffer);
#if defined(SID_PAL_LOG_RTT_ENABLED)
    SEGGER_RTT_UNLOCK();
#endif
  } else {
    sli_sid_log_print_fmt(severity, (const char *)buffer);
  }

#if defined(SL_CATALOG_SIDEWALK_LOG_APP_RTT_PRESENT)
  // CLI uses the same terminal ID as app log module
  SEGGER_RTT_SetTerminal(SL_SIDEWALK_LOG_APP_RTT_DEFAULT_TERMINAL_ID);
#else
  // app logs are over VCOM
#if defined(SID_PAL_LOG_RTT_ENABLED)
  SEGGER_RTT_SetTerminal(SL_SIDEWALK_CLI_DEFAULT_TERMINAL_ID);
#endif
#endif
}

void _app_log_time(void)
{
  app_log_append("[%08lu]" APP_LOG_SEPARATOR, get_time_now());
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static uint32_t get_time_now(void)
{
  struct sid_timespec tsp = { 0 };
  sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &tsp, NULL);
  return (tsp.tv_sec * SID_TIME_MSEC_PER_SEC  + tsp.tv_nsec / SID_TIME_NSEC_PER_MSEC);
}
