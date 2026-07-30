/***************************************************************************//**
 * @file
 * @brief Sidewalk display component configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
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

#ifndef SL_SIDEWALK_DISPLAY_CONFIG_H
#define SL_SIDEWALK_DISPLAY_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Sidewalk display configuration

// <h> Sidewalk LCD configuration

// <o> SL_SIDEWALK_DISPLAY_H_PX <0-255>
// <i> Set the LCD height pixel number
// <i> Default: 128
// <d> 128
#ifndef SL_SIDEWALK_DISPLAY_H_PX
#define SL_SIDEWALK_DISPLAY_H_PX 128
#endif

// <o> SL_SIDEWALK_DISPLAY_W_PX <0-255>
// <i> Set the LCD width pixel number
// <i> Default: 128
// <d> 128
#ifndef SL_SIDEWALK_DISPLAY_W_PX
#define SL_SIDEWALK_DISPLAY_W_PX 128
#endif

// <o> SL_SIDEWALK_DISPLAY_BORDER_PX <0-255>
// <i> Set the LCD border pixel number
// <i> Default: 5
// <d> 5
#ifndef SL_SIDEWALK_DISPLAY_BORDER_PX
#define SL_SIDEWALK_DISPLAY_BORDER_PX 5
#endif

// </h>

// <o> SL_SIDEWALK_DISPLAY_CHAR_H_PX <0-255>
// <i> Set the LCD char height pixel number
// <i> Default: 16
// <d> 16
#ifndef SL_SIDEWALK_DISPLAY_CHAR_H_PX
#define SL_SIDEWALK_DISPLAY_CHAR_H_PX 16
#endif

// <o> SL_SIDEWALK_DISPLAY_CHAR_W_PX <0-255>
// <i> Set the LCD char width pixel number
// <i> Default: 16
// <d> 16
#ifndef SL_SIDEWALK_DISPLAY_CHAR_W_PX
#define SL_SIDEWALK_DISPLAY_CHAR_W_PX 16
#endif

// <o> SL_SIDEWALK_DISPLAY_MESSAGE_START_LINE <0-255>
// <i> Set the message start line
// <i> Default: 1
// <d> 1
#ifndef SL_SIDEWALK_DISPLAY_MESSAGE_START_LINE
#define SL_SIDEWALK_DISPLAY_MESSAGE_START_LINE 1
#endif

// <o> SL_SIDEWALK_DISPLAY_STATUS_START_LINE <0-255>
// <i> Set the status start line
// <i> Default: 9
// <d> 9
#ifndef SL_SIDEWALK_DISPLAY_STATUS_START_LINE
#define SL_SIDEWALK_DISPLAY_STATUS_START_LINE 8
#endif

// </h>

// <<< end of configuration section >>>

#endif // SL_SIDEWALK_DISPLAY_CONFIG_H
