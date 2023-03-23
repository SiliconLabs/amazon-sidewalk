# Amazon Sidewalk - SoC Bluetooth Sub-GHz Hello Neighbor

The Hello Neighbor Bluetooth sub-GHz sample application leverages the Amazon Sidewalk protocol to connect to the cloud using sub-GHz FSK / CSS modulation (after an initial registration phase over BLE, if necessary). The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. The user interacts with the endpoint either by pressing the main board buttons or issuing CLI commands.

> **Ⓘ INFO Ⓘ**: This application can be used with the KG100S module or Sidewalk-supported EFR32 series 2 SoCs with at least 768kB flash size.

> **⚠ WARNING ⚠**: Sub-GHz communication occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. In order to perform these tasks and procure access to a Sidewalk gateway, complete the initial software and hardware setup steps described in [Getting Started: Prerequisites](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites).

> **Ⓘ INFO Ⓘ**: Make note of the additional sub-GHz considerations discussed in the [Silicon Labs Wireless Development Kit](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites#silicon-labs-wireless-development-kit) section of the hardware prequisites.

### Troubleshooting

In the Sidewalk Assistant, if the manufacturing page generation is stuck, you need to install the adapter pack for the Sidewalk Assistant:

- Click the **Preferences** icon on the toolbar.
- In the **Simplicity Studio** > **Adapter Packs** menu, click **Add...**.
- Browse to your freshly installed GSDK folder, then select the folder `extension/sidewalk/tools/sidewalk_assistant`.
- Click **Select Folder**.
- **Sidewalk Assistant** should appear in your adapter packs list.
- For macOS and Linux platforms you also need to give execution permissions to the binaries in `<your_gsdk_installation>/extension/sidewalk/tools/sidewalk_assistant/deploy/`.

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Prepare the Cloud and Endpoint

Create AWS resources to interface with your endpoint and couple the application image with device-specific credentials by following the steps in [Getting Started: Provision your Amazon Sidewalk Device](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/provision-your-device).

## Modulation Control

FSK is the default protocol used in the sub-GHz Hello Neighbor application. To use CSS mode (and later toggle between them), push the PB0/BTN0 button or use the CLI command described below.

## Interacting with the Endpoint

On boot, the sub-GHz radio layer (FSK or CSS) is not started - only BLE is running and will achieve time synchronization. While the sub-GHz stack is not running, all messages sent from the cloud will be received over BLE. To start the sub-GHz stack, you need to send a message to the cloud using either the push-button action or the CLI command. Once the sub-GHz stack starts, BLE time synchronization will be lost until a new connection is established with the gateway using the chosen sub-GHz protocol.

Send commands to the endpoint using either main board button presses or CLI commands.
The J-Link RTT interface provides access to the CLI commands. The following table shows the available commands and their effect, with equivalent button presses where applicable.

| Command | Description | Example | Main Board Button |
|---|---|---|---|
| update | Sends an updated counter value to the cloud | > update | PB1/BTN1 |
| switch_fsk_css | Switch between FSK/CSS modulation | > switch_fsk_css | PB0/BTN0 |
| reset | Unregisters the Sidewalk Endpoint | > reset | N/A |

## Interacting with the Cloud

Gain additional insight on network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
