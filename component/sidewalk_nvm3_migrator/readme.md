# NVM3 Migrator component

## Introduction

This component is responsible for automatically handling the changes in the NVM3 structure around the Sidewalk SDK 1.16 release. More precisely, it migrates NVM3 data from the previous unversioned Silabs-specific structrue to a new structure which also got a version number, namely 1.0.0.0. This includes migration of MFG and KV storage data and other Silabs-specific NVM3 data as well.

The goal is to provide seamless transition to the new structure by simply adding this component, and calling its single API function.

For more information about the latest NVM3 organization refer to https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-app-development/#non-volatile-memory-use
(Also find the old version here to see what changed: https://docs.silabs.com/amazon-sidewalk/1.0.1/sidewalk-developers-guide/application-development#non-volatile-memory-use)

## Usage

To use this component:
  - Add it to the project.
  - Call its API function sl_sidewalk_nvm3_migrator_run during appliation init.
    - After *sl_system_init*
    - Before *sid_platform_init*
  - There are app log messages about the migration process.

### Configuration

This component has a single configuration element, i.e., only one information is needed from the user: *SL_SIDEWALK_NVM3_MIGRATOR_ORIGINAL_DI_SIZE*

The user needs to submit the size of the NVM3 Default Instance if it has been changed before. By default the migrator component uses the most common 24576 value (24576 = 0x6000 = 24kB = 3 pages of 0x2000).

## User Requirements

There are several assumptions, i.e., requirements which shall be fulfilled by the user:
  - The orignal NVM3 data structure meet the expectation from migrator size: KV storage is placed to the end of the NVM3 area with 0x6000 size, below this (on lower address) is the MFG area with 0x6000 size and finally the Default Instance on the lowest NVM3 addresses. (See Configuration section about size.) Rationale: this expectation may seem too strict, but this NVM3 instance placement and the sizes are basically internal implementation, hidden and not configurable by the user (except the Default Instance size).
  - At the start of migration only a new application is flashed, it contains the migrator component, and the original NVM3 data is left intact. (E.g., no MFG shall be flashed beside the new application, no masserase shall be done, the new application shall not overlap with the old NVM3 area, etc.)
  - At the start of the migration the new application shall not overlap with the original NVM3 data.
  - The new application shall use the NVM3 Default Instance and put it to the end of the flash.
  - The complete original data shall fit into a 0x6000 large NVM3 instance.

## NVM3 Structure Versioning

Amazon Sidewalk enables using unique MFG object identifiers between *SID_PAL_MFG_STORE_CORE_VALUE_MAX* and *SID_PAL_MFG_STORE_VALUE_MAX* (according to the definition of *sid_pal_mfg_store_value_t*).

Therfore we added a custom MFG field with the ID 4001 and 4-byte length to contain an NVM3 structure version.
  - The migrator component adds this to the restructured NVM3 data.
  - Provisioning has been updated to add this object to the newly generated MFGs.

The up-to-date version number is 1.0.0.0.