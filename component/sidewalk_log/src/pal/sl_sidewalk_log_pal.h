/***************************************************************************//**
 * @file sl_sidewalk_log_pal.h
 * @brief sidewalk log for PAL layer
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

#ifndef SL_SIDEWALK_LOG_PAL_H
#define SL_SIDEWALK_LOG_PAL_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_sidewalk_log_core.h"
#if defined(SL_CATALOG_SIDEWALK_LOG_PAL_RTT_PRESENT)
#include "sl_sidewalk_log_pal_rtt_config.h"
#elif defined(SL_CATALOG_SIDEWALK_LOG_PAL_VCOM_PRESENT)
#include "sl_sidewalk_log_pal_vcom_config.h"
#else
#warning "No logging interface defined for PAL layer"
#endif

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/*******************************************************************************
 * Print log from the PAL layer on the chosen iostream
 *
 *  @param[in] severity Log severity
 *  @param[in] fmt Formatted string
 *  @param[in] ... Arguments to the formatted string
 ******************************************************************************/
void sl_sid_log_pal(sl_sid_log_core_log_severity_t severity, const char *fmt, ...);

/*******************************************************************************
 * Print bytes from the PAL layer on the chosen iostream
 *
 *  @param[in] severity Log severity
 *  @param[in] address Address of the data buffer to be printed
 *  @param[in] length Length of the data buffer to be printed
 ******************************************************************************/
void sl_sid_log_pal_hexdump(sl_sid_log_core_log_severity_t severity, const void *address, int length);

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#if SL_SID_LOG_PAL_ENABLED
#define SL_SID_LOG_PAL_HIGHEST_SEVERITY(level, fmt_, ...) sl_sid_log_pal(level, fmt_, ##__VA_ARGS__);
#define SL_SID_LOG_PAL(level, fmt_, ...)          \
  do {                                            \
    if (level <= SL_SID_LOG_PAL_SEVERITY) {       \
      sl_sid_log_pal(level, fmt_, ##__VA_ARGS__); \
    }                                             \
  } while (0)
#define SL_SID_LOG_PAL_HEXDUMP(level_, data_, len_) sl_sid_log_pal_hexdump(level_, data_, len_)
#else
#define SL_SID_LOG_PAL_HIGHEST_SEVERITY(level, fmt_, ...)
#define SL_SID_LOG_PAL(level_, fmt_, ...)
#define SL_SID_LOG_PAL_HEXDUMP(level_, data_, len_)
#endif

#define SL_SID_LOG_PAL_ERROR(fmt_, ...)             SL_SID_LOG_PAL_HIGHEST_SEVERITY(SL_SID_LOG_CORE_SEVERITY_ERROR, fmt_, ##__VA_ARGS__)
#define SL_SID_LOG_PAL_HEXDUMP_ERROR(data_, len_)   SL_SID_LOG_PAL_HEXDUMP(SL_SID_LOG_CORE_SEVERITY_ERROR, data_, len_)
#if SL_SID_LOG_PAL_SEVERITY >= SL_SID_LOG_CORE_SEVERITY_WARNING
#define SL_SID_LOG_PAL_WARNING(fmt_, ...)           SL_SID_LOG_PAL(SL_SID_LOG_CORE_SEVERITY_WARNING, fmt_, ##__VA_ARGS__)
#define SL_SID_LOG_PAL_HEXDUMP_WARNING(data_, len_) SL_SID_LOG_PAL_HEXDUMP(SL_SID_LOG_CORE_SEVERITY_WARNING, data_, len_)
#if SL_SID_LOG_PAL_SEVERITY >= SL_SID_LOG_CORE_SEVERITY_INFO
#define SL_SID_LOG_PAL_INFO(fmt_, ...)              SL_SID_LOG_PAL(SL_SID_LOG_CORE_SEVERITY_INFO, fmt_, ##__VA_ARGS__)
#define SL_SID_LOG_PAL_HEXDUMP_INFO(data_, len_)    SL_SID_LOG_PAL_HEXDUMP(SL_SID_LOG_CORE_SEVERITY_INFO, data_, len_)
#if SL_SID_LOG_PAL_SEVERITY >= SL_SID_LOG_CORE_SEVERITY_DEBUG
#define SL_SID_LOG_PAL_DEBUG(fmt_, ...)             SL_SID_LOG_PAL(SL_SID_LOG_CORE_SEVERITY_DEBUG, fmt_, ##__VA_ARGS__)
#define SL_SID_LOG_PAL_HEXDUMP_DEBUG(data_, len_)   SL_SID_LOG_PAL_HEXDUMP(SL_SID_LOG_CORE_SEVERITY_DEBUG, data_, len_)
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif  // SL_SIDEWALK_LOG_PAL_H
