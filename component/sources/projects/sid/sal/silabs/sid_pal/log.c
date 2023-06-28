/***************************************************************************//**
 * @file
 * @brief log.c
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
#include "em_cmu.h"
#include <sid_pal_log_ifc.h>
#include "app_log.h"
#include <stdbool.h>

#include <sid_clock_ifc.h>
#pragma message "Please note! The Sidewalk APIs from the file sid_clock_ifc.h might not be backward compatible in the future."

#include <sid_pal_uptime_ifc.h>
#include <stdarg.h>
#include "printf.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define SLI_LOG_MAX_BUFFER_CHAR  256

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
uint64_t silabs_get_rail_time_64(void);

void sid_pal_log_flush(void)
{
  // Our platform logging functionality does not need flushing
}

char const *sid_pal_log_push_str(char *string)
{
  return (char const *)string;
}

/*******************************************************************************
 * Printf style logging function
 *
 * @param[in]   serverity       Severity of the log
 * @param[in]   num_args        Number of arguments to be logged
 * @param[in]   fmt             Format string to print with variables
 ******************************************************************************/
void sid_pal_log(sid_pal_log_severity_t severity,
                 uint32_t num_args,
                 const char * fmt,
                 ...)
{
  (void)num_args;
  char buffer[SLI_LOG_MAX_BUFFER_CHAR];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, SLI_LOG_MAX_BUFFER_CHAR, fmt, args);
  va_end(args);

  switch (severity) {
    case SID_PAL_LOG_SEVERITY_ERROR:
      app_log_error(buffer);
      break;
    case SID_PAL_LOG_SEVERITY_WARNING:
      app_log_warning(buffer);
      break;
    case SID_PAL_LOG_SEVERITY_INFO:
      app_log_info(buffer);
      break;
    case SID_PAL_LOG_SEVERITY_DEBUG:
      app_log_debug(buffer);
      break;
  }
}

bool sid_pal_log_get_log_buffer(struct sid_pal_log_buffer *const log_buffer)
{
  (void)log_buffer;
  return false;
}

void sid_pal_hexdump(sid_pal_log_severity_t severity, const void *address, int length)
{
#if SID_PAL_LOG_ENABLED
  if (severity <= SID_PAL_LOG_LEVEL) {
    char const digit[16] = "0123456789ABCDEF";
    uint8_t idx = 0;
    char hex_buf[SID_PAL_HEXDUMP_MAX * 3 + 1] = { 0 };
    uint8_t *data = (uint8_t *)address;
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
#endif
}

uint32_t get_time_now(void)
{
  struct sid_timespec tsp = { 0 };

  sid_clock_now(SID_CLOCK_SOURCE_UPTIME, &tsp, NULL);
  return (tsp.tv_sec * SID_TIME_MSEC_PER_SEC  + tsp.tv_nsec / SID_TIME_NSEC_PER_MSEC);
}

/*******************************************************************************
 * Override Weak declaration of _app_log_time in Silabs SDK so that the time
 * display is compatible with other vendors
 *
 ******************************************************************************/
void _app_log_time()
{
  app_log_append("[%08lu]" APP_LOG_SEPARATOR, get_time_now());
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
