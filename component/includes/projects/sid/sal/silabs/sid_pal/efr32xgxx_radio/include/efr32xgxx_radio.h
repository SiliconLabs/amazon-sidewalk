/***************************************************************************//**
 * @file
 * @brief efr32xgxx_radio.h
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

#ifndef EFR32XGXX_RADIO_H
#define EFR32XGXX_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <efr32xgxx_config.h>
#include <sid_pal_radio_ifc.h>

#include "silabs/efr32xgxx.h"

#include <stdint.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
typedef struct {
  const radio_efr32xgxx_device_config_t        *config;
  sid_pal_radio_modem_mode_t                   modem;
  sid_pal_radio_rx_packet_t                    *radio_rx_packet;
  sid_pal_radio_event_notify_t                 report_radio_event;
  sid_pal_radio_irq_handler_t                  irq_handler;
  uint8_t                                      radio_state;
  sid_pal_radio_cad_param_exit_mode_t          cad_exit_mode;
  uint32_t                                     radio_freq_hz;
  radio_efr32xgxx_regional_param_t             regional_radio_param;
} halo_drv_silabs_ctx_t;

#define US_IN_SEC                                      (1000000UL)
#define EFR32XGXX_US_TO_SYMBOLS(time_in_us, bit_rate)  ((time_in_us * bit_rate) / US_IN_SEC)

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
halo_drv_silabs_ctx_t* efr32xgxx_get_drv_ctx(void);

int32_t radio_fsk_process_rx_done(const halo_drv_silabs_ctx_t *drv_ctx);

#ifdef __cplusplus
}
#endif

#endif //EFR32XGXX_RADIO_H
