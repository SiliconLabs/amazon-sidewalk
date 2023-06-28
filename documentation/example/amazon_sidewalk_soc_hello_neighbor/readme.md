# Amazon Sidewalk - SoC Bluetooth Sub-GHz Hello Neighbor

The Hello Neighbor sample application leverages the Amazon Sidewalk protocol to connect to the cloud using either BLE or sub-GHz FSK / CSS modulation (after an initial registration phase over BLE, if necessary). The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. The user interacts with the endpoint either by pressing the main board buttons (not supported when using KG100S) or issuing CLI commands.

> **Ⓘ INFO Ⓘ**: This application can be used with the KG100S module or Sidewalk-supported EFR32 series 2 SoCs with at least 768kB flash size.

> **⚠ WARNING ⚠**: Sub-GHz communication occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. In order to perform these tasks and procure access to a Sidewalk gateway, complete the initial software and hardware setup steps described in [Getting Started: Prerequisites](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites).

> **Ⓘ INFO Ⓘ**: Make note of the additional sub-GHz considerations discussed in the [Silicon Labs Wireless Development Kit](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites#silicon-labs-wireless-development-kit) section of the hardware prequisites.

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Prepare the Cloud and Endpoint

Create AWS resources to interface with your endpoint and couple the application image with device-specific credentials by following the steps in [Getting Started: Provision your Amazon Sidewalk Device](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/provision-your-device).

## Modulation Control

By default, the Hello Neighbor application will only start on a default radio layer (BLE for most boards). If you wish to change the default radio layer you can modify the `SL_SIDEWALK_COMMON_DEFAULT_LINK_TYPE` define value in file `config/sl_sidewalk_common_config.h`. Possible values are:

| Value | Radio Layer | Comment |
|---|---|---|
| SL_SIDEWALK_LINK_BLE | BLE |  |
| SL_SIDEWALK_LINK_FSK | FSK |  |
| SL_SIDEWALK_LINK_CSS | CSS | Registration is not supported on CSS, you first need to register using either BLE or FSK sample app. |

On first boot with CSS modulation, the device will start on either FSK or BLE (depending on device support) to perform registration and switch back to CSS once registration is valid.

## Interacting with the Endpoint

On boot, the chosen radio layer will be selected. However, to start the sub-GHz (FSK or CSS) stack, you must first send a message to the cloud using either the `update` or `send` commands in the table below.

Send commands to the endpoint using either main board button presses or CLI commands. Note that on KG100S only the CLI commands are available.
The J-Link RTT interface provides access to the CLI commands. The following table shows the available commands and their effect, with equivalent button presses where applicable.

| Command | Description | Example | Main Board Button |
|---|---|---|---|
| switch_fsk_css | Switch between FSK/CSS modulation (only available if running link is either FSK or CSS) | > switch_fsk_css | PB0/BTN0 |
| connect | Connects the endpoint to a Sidewalk gateway (only available in BLE) | > connect | PB0/BTN0 |
| update | Sends an updated counter value to the cloud | > update | PB1/BTN1 |
| send | Sends an ASCII string of length of N to the cloud | > send N | N/A |
| reset | Unregisters the Sidewalk Endpoint | > reset | N/A |

> **⚠ WARNING ⚠**: The `send` command may crash if no argument or too small, large argument is provided. Size (N) must be in the valid range of [1, 20].

> **⚠ WARNING ⚠**: The `reset` command is used to unregister your device with the cloud. It can only be called on a registered AND time synced device.

## Interacting with the Cloud

Gain additional insight on network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
