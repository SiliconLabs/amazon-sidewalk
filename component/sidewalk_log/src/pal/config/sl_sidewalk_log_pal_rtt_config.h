/***************************************************************************//**
 * @file sl_sidewalk_log_pal_rtt_config.h
 * @brief sidewalk RTT log configuration for PAL layer usage
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

#ifndef SL_SIDEWALK_LOG_PAL_RTT_CONFIG_H
#define SL_SIDEWALK_LOG_PAL_RTT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_sidewalk_log_core.h"

// <<< Use Configuration Wizard in Context Menu >>>

// <e SL_SID_LOG_PAL_ENABLED> PAL logs
#define SL_SID_LOG_PAL_ENABLED 0

// <o SL_SIDEWALK_LOG_PAL_RTT_DEFAULT_TERMINAL_ID> Terminal ID
// <d> 2
#define SL_SIDEWALK_LOG_PAL_RTT_DEFAULT_TERMINAL_ID 2

// <o SL_SID_LOG_PAL_SEVERITY> Severity
// <SL_SID_LOG_CORE_SEVERITY_DEBUG=> DEBUG
// <SL_SID_LOG_CORE_SEVERITY_INFO=> INFO
// <SL_SID_LOG_CORE_SEVERITY_WARNING=> WARNING
// <SL_SID_LOG_CORE_SEVERITY_ERROR=> ERROR
#define SL_SID_LOG_PAL_SEVERITY SL_SID_LOG_CORE_SEVERITY_INFO

// </e>

// <<< end of configuration section >>>

#ifdef __cplusplus
}
#endif

#endif  // SL_SIDEWALK_LOG_PAL_RTT_CONFIG_H
