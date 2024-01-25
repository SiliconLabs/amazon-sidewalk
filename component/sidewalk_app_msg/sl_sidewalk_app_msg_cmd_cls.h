/***************************************************************************//**
 * @file sl_sidewalk_app_msg_cmd_cls.h
 * @brief sidewalk application message component
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

#ifndef SL_SID_APP_MSG_CMD_CLS_H
#define SL_SID_APP_MSG_CMD_CLS_H

/*******************************************************************************
 *** MACROS AND TYPEDEFS
 ******************************************************************************/

// Command class (encoded in 4 bits)
// New command classes can be added into this list
enum sli_sid_app_msg_cmd_cls {
  SLI_SID_APP_MSG_CMD_CLS_DEV_MGMT = 0,   // device management
  SLI_SID_APP_MSG_CMD_CLS_CLOUD_MGMT,     // cloud management
  SLI_SID_APP_MSG_CMD_CLS_SID,            // sidewalk specific
  SLI_SID_APP_MSG_CMD_CLS_DMP_SOC_LIGHT   // DMP SOC Light application
};

#endif  // SL_SID_APP_MSG_CMD_CLS_H
