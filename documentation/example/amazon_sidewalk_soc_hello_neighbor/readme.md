# Amazon Sidewalk - SoC Bluetooth Sub-GHz Hello Neighbor

The Hello Neighbor sample application leverages the Amazon Sidewalk protocol to connect to the cloud using either BLE or sub-GHz FSK / CSS modulation (after an initial registration phase over BLE, if necessary). The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. The user interacts with the endpoint either by pressing the main board buttons (not supported when using KG100S) or issuing CLI commands.

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

By default, the Hello Neighbor application will only start on a default radio layer (BLE for most boards). If you wish to change the default radio layer you can modify the `SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE` define value in the `config/sl_sidewalk_common_config.h` file. Possible values are:

| Value | Radio Layer | Comment |
|---|---|---|
| SL_SIDEWALK_LINK_BLE | BLE |  |
| SL_SIDEWALK_LINK_FSK | FSK |  |
| SL_SIDEWALK_LINK_CSS | CSS | Registration is not supported on CSS, you first need to register using either BLE or FSK sample app. |

On the first boot with CSS modulation, the device will start on either FSK or BLE (depending on device support) to perform registration and switch back to CSS once registration is valid.

> **⚠ WARNING ⚠**: All radio layer might not be supported by the radio board you are using, in which case the unsupported radio layers are slipped while switching. In case of single radio layer support (eg. xG23 family), the switch action is not available.

## Interacting with the Endpoint

On boot, the chosen radio layer will be selected. However, to start the sub-GHz (FSK or CSS) stack, you must first send a message to the cloud using either the `update` or `send` commands in the table below.

Send commands to the endpoint using either the main board button presses or CLI commands. Note that on KG100S the button actions are different because only one button is available.
The J-Link RTT interface provides access to the CLI commands. The following table shows the available commands and their effect, with equivalent button presses where applicable. Some commands are dependent on the currently selected radio layer.

### All radio boards push-button actions:

| Command | Description | Example | Main Board Button |
|---|---|---|---|
| switch_link | Switch between BLE, FSK and CSS modulation (depending on supported radio, switch order is BLE->FSK->CSS) | > switch_link | PB0/BTN0 |
| send | Connects to GW (BLE only) and sends an updated counter value to the cloud | > send | PB1/BTN1 |
| reset | Unregisters the Sidewalk Endpoint | > reset | N/A |

### KG100S push-button actions:

| Command | Description | Example | Main Board Button |
|---|---|---|---|
| switch_link | Switch between BLE, FSK and CSS modulation (depending on supported radio, switch order is BLE->FSK->CSS) | > switch_link | PB0/BTN0 (long-press) |
| send | Connects to GW (BLE only) and sends an updated counter value to the cloud | > send | PB0/BTN0 |
| reset | Unregisters the Sidewalk Endpoint | > reset | N/A |

> **⚠ WARNING ⚠**: The `reset` command is used to unregister your device with the cloud. It can only be called on a registered AND time synced device.

## Interacting with the Cloud

Gain additional insight on the network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
