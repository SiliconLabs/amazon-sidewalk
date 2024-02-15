Silicon Labs Amazon Sidewalk Release Note
=========================================

# Release 2.0.1
(release date 2024-02-14)

*GSDK 4.4.1 - Amazon Sidewalk SDK 1.16.2*

## New Features/Improvements
### Silicon Labs Extension
- Support for GSDK 4.4.1

### Amazon Sidewalk stack
- Include Amazon Sidewalk SDK 1.16.2

## Known issues
- ~~[1148] - In CSS, if the gateway is not available, endpoint does not exponentially backoff for time_sync request and sends a new time_sync request every 30secs forever. Workaround in place in application code, to stop time sync after timeout.~~
- ~~[1253713] - Cloudformation could not be deployed due to S3 bucket resource name was fixed. It is solved via generation of the S3 bucket name based on the AWs stackid.~~
- [1174] - On KG100S, we see errors related to missing acknowledge `tx_st:-6` with the CLI application. However, the stack recovers after some time.
- [47374] - Message duplicate error during FFN: `<error> msg dup: xxxxxxxxx (xxx)`. Device recovers in next FFN session.

### Sidewalk Assistant Known issues
- ~~Sidewalk Assistant does not generate the manufacturing page on Windows.~~
- Removed the Application tab in the Sidewalk Assistant.
- Sidewalk Assistant allows underscores in cloud formation stack name, however this is not accepted by AWS.
- Sidewalk Assistant is not able to detect AWS credentials if it is not in the default AWS profile.
- On MAC OS with Sidewalk Assistant, changing the Cloud Formation name on the GUI will not change it in the cloud.

# Release 2.0.0
(release date 2024-01-24)

*GSDK 4.4.0 - Amazon Sidewalk SDK 1.16.0.2*

## New Features/Improvements
### Silicon Labs Extension
- Support for GSDK 4.4.0
- Include Amazon Sidewalk SDK 1.16.0.2
- NVM3 rework to free space, MFG page changed location (breaking backward compatibility)
- Semtech PAL layer implementation open sourced
- TLV message structure added as sotware component
- SWI as software component: IRQ or thread method can be selected
- New sample applications:
  - Dynamic Multiprotocol for xG28 (as Evaluation)
  - Production Device Provisioner (as Production)

### Amazon Sidewalk stack
- Multilink support [Application note](https://docs.sidewalk.amazon/assets/pdf/Amazon_Sidewalk_Multi-link_App_Note-1.0-rev-A.pdf)
- Bulk data transfer

> **⚠ WARNING ⚠**: NVM3 organisation changed, breaking backward compatibility. Manufacturing pages should be re-generated and flashed again.

## Known issues
- ~~[1090] Switching to UART logs does no longer fails FSK FFN registration~~
- [1148] - In CSS, if the gateway is not available, endpoint does not exponentially backoff for time_sync request and sends a new time_sync request every 30secs forever. Workaround in place in application code, to stop time sync after timeout.
- [1174] - On KG100S, we see errors related to missing acknowledge `tx_st:-6` with the CLI application. However, the stack recovers after some time.
- [47374] - Message duplicate error during FFN: `<error> msg dup: xxxxxxxxx (xxx)`. Device recovers in next FFN session.

### Sidewalk Assistant Known issues
- Sidewalk Assistant does not generate the manufacturing page on Windows.

# Release 1.2.2
(release date 2023-10-17)

*GSDK 4.3.2 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Support for GSDK 4.3.2
- xG24 CSP support
- The OOB SW components is shared in source

## Known issues
- [1090] Switching to UART logs does no longer fails FSK FFN registration

### Sidewalk Assistant Known issues
- Sidewalk Assistant does not generate the manufacturing page on Windows.

# Release 1.2.1
(release date 2023-10-13)

*GSDK 4.3.1 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Source files of manufacturing page generator are added
- Bugfix for manufacturing page generator

## Known issues
- [1090] - For xG24, xG21 and KG100S, switching to UART logs fails FSK FFN registration.

# Release 1.2.0
(release date 2023-08-11)

*GSDK 4.3.1 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Support for GSDK 4.3.1
- EFR32xG23 support
- EFR32xG28 support
- EFR32xG24 Dev Kit support
- Button handling rework
- Sidewalk CLI sample application improvements

# Release 1.1.0
(release date 2023-06-28)

*GSDK 4.3.0 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Support for GSDK 4.3.0
- PAL source files open sourced
- The BLE and Sub-GHz Hello Neighbor sample applications are merged into one
- The BLE and Sub-GHz Sidewalk CLI sample applications are merged into one

# Release 1.0.3
(release date 2023-06-14)

*GSDK 4.2.3 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Support for GSDK 4.2.3

# Release 1.0.2
(release date 2023-04-28)

*GSDK 4.2.2 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Out Of the Box demo files added

# Release 1.0.1
(release date 2023-14-10)

*GSDK 4.2.2 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- License update

# Release 1.0.0
(release date 2023-03-28)

*GSDK 4.2.2 - Amazon Sidewalk SDK 1.14*

## New Features/Improvements
- Support for GSDK 4.2.2
- Include Amazon Sidewalk SDK 1.14