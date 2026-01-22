/***************************************************************************//**
 * @file sl_sidewalk_log_app.c
 * @brief sidewalk log for APP layer
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
#include "sl_sidewalk_log_app.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define LOG_MAX_HEXDUMP_CHAR (8)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static sl_iostream_t *get_iostream(void);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void sl_sid_log_app(sl_sid_log_core_log_severity_t severity, const char *fmt, ...)
{
  sl_iostream_t *iostream = get_iostream();
  if (iostream == NULL) {
    return;
  }

  sl_sid_log_core_set_iostream(iostream);
#if defined(SL_CATALOG_SIDEWALK_LOG_APP_RTT_PRESENT)
  sl_sid_log_core_set_rtt_terminal_id(SL_SIDEWALK_LOG_APP_RTT_DEFAULT_TERMINAL_ID);
#endif

  va_list args;
  va_start(args, fmt);
  sl_sid_log_core_print(severity, fmt, args);
  va_end(args);
}

void sl_sid_log_app_hexdump(sl_sid_log_core_log_severity_t severity, const void *address, int length)
{
  if (severity <= SL_SID_LOG_APP_SEVERITY) {
    char const digit[16] = "0123456789ABCDEF";
    uint8_t idx = 0;
    char hex_buf[LOG_MAX_HEXDUMP_CHAR * 3 + 1] = { 0 };
    const uint8_t *data = (const uint8_t *)address;
    for (int i = 0; i < length; i++) {
      if (idx && ((i % LOG_MAX_HEXDUMP_CHAR) == 0)) {
        SL_SID_LOG_APP(severity, "%s", hex_buf);
        idx = 0;
      }
      hex_buf[idx++] = digit[(data[i] >> 4) & 0x0f];
      hex_buf[idx++] = digit[(data[i] >> 0) & 0x0f];
      hex_buf[idx++] = ' ';
      hex_buf[idx] = '\0';
    }
    if (idx) {
      SL_SID_LOG_APP(severity, "%s", hex_buf);
    }
  }
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sl_iostream_t *get_iostream(void)
{
  sl_iostream_t *iostream;
#if defined(SL_CATALOG_SIDEWALK_LOG_APP_RTT_PRESENT)
  iostream = sl_iostream_get_handle("rtt");
#elif defined(SL_CATALOG_SIDEWALK_LOG_APP_VCOM_PRESENT)
  iostream = sl_iostream_get_handle(SL_SID_LOG_APP_VCOM_INSTANCE_NAME);
#else
  iostream = NULL;
#endif
  return iostream;
}
