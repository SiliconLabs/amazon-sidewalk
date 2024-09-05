/***************************************************************************//**
 * @file sl_sidewalk_log_stack.c
 * @brief sidewalk log for stack layer
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

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include "sl_sidewalk_log_core.h"
#include "sid_pal_log_ifc.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static sl_iostream_t *get_iostream(void);

static sl_sid_log_core_log_severity_t map_severity(sid_pal_log_severity_t severity);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sid_pal_log(sid_pal_log_severity_t severity, uint32_t num_args, const char *fmt, ...)
{
  (void)num_args;

#if SID_PAL_LOG_ENABLED
  sl_iostream_t *iostream = get_iostream();
  if (iostream == NULL) {
    return;
  }

  sl_sid_log_core_set_iostream(iostream);
#if defined(SL_CATALOG_SIDEWALK_LOG_STACK_RTT_PRESENT)
  sl_sid_log_core_set_rtt_terminal_id(SL_SIDEWALK_LOG_STACK_RTT_DEFAULT_TERMINAL_ID);
#endif

  va_list args;
  va_start(args, fmt);
  sl_sid_log_core_print(map_severity(severity), fmt, args);
  va_end(args);
#else
  (void)severity;
  (void)fmt;
#endif
}

bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer)
{
  (void)log_buffer;
  return false;
}

void sid_pal_hexdump(sid_pal_log_severity_t severity, const void *address, int length)
{
  if (severity <= SID_PAL_LOG_LEVEL) {
    char const digit[16] = "0123456789ABCDEF";
    uint8_t idx = 0;
    char hex_buf[SID_PAL_HEXDUMP_MAX * 3 + 1] = { 0 };
    const uint8_t *data = (const uint8_t *)address;
    for (int i = 0; i < length; i++) {
      if (idx && ((i % SID_PAL_HEXDUMP_MAX) == 0)) {
        SID_PAL_LOG(severity, "%s", SID_PAL_LOG_PUSH_STR(hex_buf));
        idx = 0;
      }
      hex_buf[idx++] = digit[(data[i] >> 4) & 0x0f];
      hex_buf[idx++] = digit[(data[i] >> 0) & 0x0f];
      hex_buf[idx++] = ' ';
      hex_buf[idx] = '\0';
    }
    if (idx) {
      SID_PAL_LOG(severity, "%s", SID_PAL_LOG_PUSH_STR(hex_buf));
    }
  }
}

void sid_pal_log_flush(void)
{
  // Silabs platform logging functionality does not need flushing
}

char const *sid_pal_log_push_str(char *string)
{
  return (char const *)string;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static sl_iostream_t *get_iostream(void)
{
  sl_iostream_t *iostream;
#if defined(SL_CATALOG_SIDEWALK_LOG_STACK_RTT_PRESENT)
  iostream = sl_iostream_get_handle("rtt");
#elif defined(SL_CATALOG_SIDEWALK_LOG_STACK_VCOM_PRESENT)
  iostream = sl_iostream_get_handle(SL_SID_LOG_STACK_VCOM_INSTANCE_NAME);
#else
  iostream = NULL;
#endif
  return iostream;
}

static sl_sid_log_core_log_severity_t map_severity(sid_pal_log_severity_t severity)
{
  switch (severity) {
    case SID_PAL_LOG_SEVERITY_ERROR:
      return SL_SID_LOG_CORE_SEVERITY_ERROR;
    case SID_PAL_LOG_SEVERITY_WARNING:
      return SL_SID_LOG_CORE_SEVERITY_WARNING;
    case SID_PAL_LOG_SEVERITY_INFO:
      return SL_SID_LOG_CORE_SEVERITY_INFO;
    case SID_PAL_LOG_SEVERITY_DEBUG:
      return SL_SID_LOG_CORE_SEVERITY_DEBUG;
    default:
      return SL_SID_LOG_CORE_SEVERITY_ERROR;
  }
}
