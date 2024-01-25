/***************************************************************************//**
 * @file
 * @brief efr32xgxx_radio_lora.c
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
#include "efr32xgxx_radio.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

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
sid_pal_radio_data_rate_t sid_pal_radio_lora_mod_params_to_data_rate(const sid_pal_radio_lora_modulation_params_t *mod_params)
{
  (void)mod_params;
  return SID_PAL_RADIO_DATA_RATE_INVALID;
}

int32_t sid_pal_radio_set_lora_sync_word(uint16_t sync_word)
{
  (void)sync_word;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_symbol_timeout(uint8_t num_of_symbols)
{
  (void)num_of_symbols;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_modulation_params(const sid_pal_radio_lora_modulation_params_t *mod_params)
{
  (void)mod_params;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_packet_params(const sid_pal_radio_lora_packet_params_t *packet_params)
{
  (void)packet_params;
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_set_lora_cad_params(const sid_pal_radio_lora_cad_params_t *cad_params)
{
  (void)cad_params;
  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_lora_time_on_air(const sid_pal_radio_lora_modulation_params_t *mod_params,
                                        const sid_pal_radio_lora_packet_params_t *packet_params,
                                        uint8_t packet_len)
{
  (void)mod_params;
  (void)packet_params;
  (void)packet_len;
  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_get_lora_symbol_timeout_us(sid_pal_radio_lora_modulation_params_t *mod_params, uint8_t number_of_symbol)
{
  (void)mod_params;
  (void)number_of_symbol;
  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_get_lora_tx_process_delay(void)
{
  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_get_lora_rx_process_delay(void)
{
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_lora_start_cad(void)
{
  return RADIO_ERROR_NONE;
}

int32_t sid_pal_radio_lora_data_rate_to_mod_params(sid_pal_radio_lora_modulation_params_t *mod_params,
                                                   sid_pal_radio_data_rate_t data_rate,
                                                   uint8_t li_enable)
{
  (void)mod_params;
  (void)data_rate;
  (void)li_enable;
  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_lora_get_lora_number_of_symbols(const sid_pal_radio_lora_modulation_params_t *mod_params,
                                                       uint32_t delay_micro_sec)
{
  (void)mod_params;
  (void)delay_micro_sec;
  return RADIO_ERROR_NONE;
}

uint32_t sid_pal_radio_lora_cad_duration(uint8_t symbol, const sid_pal_radio_lora_modulation_params_t *mod_params)
{
  (void)symbol;
  (void)mod_params;
  return RADIO_ERROR_NONE;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
