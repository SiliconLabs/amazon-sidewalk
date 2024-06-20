/***************************************************************************//**
 * @file sl_sidewalk_pal_btl_ifc.h
 * @brief sidewalk bootloader interface pal component
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

#ifndef SL_SIDEWALK_PAL_BTL_IFC_H
#define SL_SIDEWALK_PAL_BTL_IFC_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * @brief
 *   Initializes gecko bootloader and bootloader interface for DFU OTA.
 ******************************************************************************/
void sid_pal_btl_ifc_init(void);

/***************************************************************************//**
 * @brief
 *   Gets the available memory to be used by DFU OTA process.
 *
 * @return Available memory
 ******************************************************************************/
uint32_t sid_pal_btl_ifc_get_free_space(void);

/***************************************************************************//**
 * @brief
 *   Writes data onto the memory.
 *
 * @param[in] offset Offset from the start addres of bootloader slot 0
 * @param[in] data Pointer to the data to be written
 * @param[in] data_len Data length
 * 
 * @return true for success and false for failure
 ******************************************************************************/
bool sid_pal_btl_ifc_write(uint32_t offset, void *data, uint32_t data_len);

/***************************************************************************//**
 * @brief
 *   Checks the CRC and triggers the bootloader to update the firmware.
 *
 * @param[in] data_len Data length over which the CRC will be computed
 * @param[in] expected_crc Expected CRC
 * 
 * @return true for success and false for failure
 ******************************************************************************/
bool sid_pal_btl_ifc_finalize(uint32_t data_len, uint32_t expected_crc);

/***************************************************************************//**
 * @brief
 *   Reboots system in REBOOT_RESET_TIMER_VALUE seconds.
 ******************************************************************************/
void sid_pal_btl_ifc_reboot(void);

/***************************************************************************//**
 * @brief
 *   Computes the CRC based on the previous one.
 *
 * @param[in] p_data Data
 * @param[in] size Data length
 * @param[in] p_crc Previous CRC
 * 
 * @return true for success and false for failure
 ******************************************************************************/
uint32_t sid_pal_btl_ifc_crc32_compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc);

#ifdef __cplusplus
}
#endif

#endif  // SL_SIDEWALK_PAL_BTL_IFC_H
