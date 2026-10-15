# Amazon Sidewalk - SoC Location

The Location sample application leverages the Amazon Sidewalk protocol to connect to the cloud using either BLE or sub-GHz FSK / CSS modulation (after an initial registration phase over BLE, if necessary). The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. It also integrates the Amazon Location Services support in Amazon Sidewalk. The user interacts to try location features by issuing CLI commands.

> **Ⓘ INFO Ⓘ**: This application can be used with the KG100S module or Sidewalk-supported EFR32 series 2 SoCs with at least 768kB flash size.

> **⚠ WARNING ⚠**: Sub-GHz communication occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. To perform these tasks and procure access to a Sidewalk gateway, complete the initial software and hardware setup steps described in [Getting Started: Prerequisites](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites).

> **Ⓘ INFO Ⓘ**: Make note of the additional sub-GHz considerations discussed in the [Silicon Labs Wireless Development Kit](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites#silicon-labs-wireless-development-kit) section of the hardware prequisites.

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Prepare the Cloud and Endpoint

Create AWS resources to interface with your endpoint and couple the application image with device-specific credentials by following the steps in [Getting Started: Provision your Amazon Sidewalk Device](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/provision-your-device).

## Modulation Control

By default, the Location application will only start on a default radio layer (BLE for most boards). If you wish to change the default radio layer you can modify the `SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE` define value in the `config/sl_sidewalk_common_config.h` file. Possible values are:

| Value | Radio Layer | Comment |
|---|---|---|
| SL_SIDEWALK_LINK_BLE | BLE |  |
| SL_SIDEWALK_LINK_FSK | FSK |  |
| SL_SIDEWALK_LINK_CSS | CSS | Registration is not supported on CSS, you first need to register using either BLE or FSK sample app. |

On the first boot with CSS modulation, the device will start on either FSK or BLE (depending on device support) to perform registration and switch back to CSS once registration is valid.

> **⚠ WARNING ⚠**: All radio layer might not be supported by the radio board you are using, in which case the unsupported radio layers are slipped while switching. In case of single radio layer support (eg. xG23 family), the switch action is not available.

## Interacting with the Endpoint

Amazon Sidewalk supports 3 location methods: Sidewalk Network Location over BLE, WiFi Scan and GNSS Scan. Different effort levels are defined in the stack to choose which location methods can be used:

- Effort level 1 corresponds to BLE only based location.
- Effort level 3 corresponds to WiFi and BLE based location depending on what is available around the device at the time of the scan.
- Effort level 4 corresponds to GNSS, WiFi and BLE based location depending on what is available around the device at the time of the scan.

Effort level 1 requires the Sidewalk link to be BLE. Effort levels 3 and 4 can be used alongside any running Sidewalk radio link. For example, you can call for a level 3 positioning while the device is running BLE.

To see the available commands, enter the following command in the console:

`help`

The list of available commands is output on the console with the associated help. Following is an extended description of and examples of how to use each command.

| Command | Description | Example |
|---|---|---|
| location_init | Initialize the location services on the endpoint. | > location_init |
| location_deinit | Deinitialize the location services on the endpoint. | > location_deinit |
| location_send \<effort level\> | Starts a location scan and immediately sends the result to the cloud with the selected effort level (BLE: 1, Wi-Fi: 3, GNSS: 4) | > location_send 4 |
| location_scan \<effort level\> | Starts a location scan and stores the result to be sent later with the selected effort level (BLE: 1, Wi-Fi: 3, GNSS: 4) | > location_scan 4 |
| location_send_buf \<effort level\> | Sends a previously saved location scan result to the cloud with the selected effort level (BLE: 1, Wi-Fi: 3, GNSS: 4) | > location_send_buf 4 |
| location_alm_start | Start the almanac demodulation service to update over satellite (GNSS only) | > location_alm_start |
| ble_connect | Connect the BLE link to the gateway, this is necessary to send data (BLE only) | > ble_connect |
| switch_link | Switch between BLE, FSK and CSS modulation (depending on supported radio, switch order is BLE->FSK->CSS) | > switch_link |
| send | Connects to GW (BLE only) and sends an updated counter value to the cloud | > send |
| reset | Unregisters the Sidewalk Endpoint | > reset |

## Interacting with the Cloud

To use the location services in the cloud, your device must have location enabled in the cloud as well. You can verify this in the AWS IoT Wireless console by opening your Sidewalk device's details page.

To activate the positioning feature:

1. Click **Edit** on the device details page.
2. Scroll down to the **Device Position Configuration** section.
3. Under **Geolocation**, check **Activate positioning**.
4. In **Position data destination**, choose the destination where you want AWS to send the device’s location information.
   - This rule defines how the location payload will be processed in your cloud application.

Make sure to save your changes before leaving the page.

Gain additional insight on the network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).