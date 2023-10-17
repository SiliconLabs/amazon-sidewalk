/***************************************************************************//**
 * @file
 * @brief sl_sidewalk_qr_code.c
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sli_sidewalk_qr_code.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void generate_qr_code(uint8_t *qr_buffer, char *text_to_encode);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

void sli_sidewalk_qr_code_create_qr_from_str(sli_sidewalk_qr_code_qr_t *qr_buffer, char *string)
{
  if (qr_buffer != NULL && string != NULL) {
    generate_qr_code(qr_buffer->buffer, string);
    qr_buffer->width = qrcodegen_getSize(qr_buffer->buffer);
  }
}

bool sli_sidewalk_qr_code_is_module_pixel_dark(sli_sidewalk_qr_code_qr_t *qr_buffer, uint8_t x, uint8_t y)
{
  if (qr_buffer != NULL) {
    if (true == qrcodegen_getModule(qr_buffer->buffer, x, y)) {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

static void generate_qr_code(uint8_t *qr_buffer, char *text_to_encode)
{
  /* Too big for the stack, that is why it is static*/
  static uint8_t qr_code_tmp[qrcodegen_BUFFER_LEN_MAX];

  qrcodegen_encodeText(text_to_encode,
                       qr_code_tmp,
                       qr_buffer,
                       qrcodegen_Ecc_LOW,
                       qrcodegen_VERSION_MIN,
                       qrcodegen_VERSION_MAX,
                       qrcodegen_Mask_AUTO,
                       true);
}
