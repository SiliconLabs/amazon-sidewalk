/*
 * Copyright 2020-2024 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.  This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "application_properties.h"

/// Version number for this application (uint32_t)
#define APP_PROPERTIES_VERSION 1

/// Unique ID (e.g. UUID or GUID) for the product this application
/// is built for (uint8_t[16])
#define APP_PROPERTIES_ID { 0 }

#define KEEP_SYMBOL           __attribute__((used))
KEEP_SYMBOL const ApplicationProperties_t sl_app_properties = {
  /// @brief Magic value indicating that this is an ApplicationProperties_t
  /// Must equal @ref APPLICATION_PROPERTIES_MAGIC
  .magic = APPLICATION_PROPERTIES_MAGIC,

  /// Version number of this struct
  .structVersion = APPLICATION_PROPERTIES_VERSION,

  /// Type of signature this application is signed with
  .signatureType = APPLICATION_SIGNATURE_NONE,

  /// Location of the signature. Typically a pointer to the end of application
  .signatureLocation = 0,

  /// Information about the application
  .app = {
    /// Bitfield representing type of application
    /// e.g. @ref APPLICATION_TYPE_BLUETOOTH_APP
    .type = APPLICATION_TYPE_FLEX,

    /// Version number for this application
    .version = APP_PROPERTIES_VERSION,

    /// Capabilities of this application
    .capabilities = 0,

    /// Unique ID (e.g. UUID/GUID) for the product this application is built for
    .productId = APP_PROPERTIES_ID,
  },
};
