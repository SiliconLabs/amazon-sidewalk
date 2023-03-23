/***************************************************************************//**
 * @file
 * @brief SX126x Config
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SX126X_CONFIG_H
#define SL_SX126X_CONFIG_H
/*
 * Configuration of the pin used to communicate with sx126x chip
 * (semtech LORA radio) through expension board (BRD8042A - Rev 0.2)
 */


// BUSY on PD03
// Used to indicate the status of internal state machine
#define SL_BUSY_PIN                              3
#define SL_BUSY_PORT                             gpioPortD

// ANT_SW on PD02
// External antenna switch to control antenna switch to RECEIVE or
// TRANSMIT.
#define SL_ANTSW_PIN                             2
#define SL_ANTSW_PORT                            gpioPortD

// DIO1 on PA05
// IRQ line from sx126x chip
// See sx126x datasheet for IRQs list.
#define SL_DIO_PIN                               5
#define SL_DIO_PORT                              gpioPortA

// SX NRESET on PA06
// Factory reset pin. Will be followed by standard calibration procedure
// and previous context will be lost.
#define SL_NRESET_PIN                            6
#define SL_NRESET_PORT                           gpioPortA

#endif // SL_SX126X_CONFIG_H

