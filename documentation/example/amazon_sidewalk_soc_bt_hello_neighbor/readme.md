# Amazon Sidewalk - SoC Bluetooth Hello Neighbor

The Hello Neighbor (Bluetooth) sample application leverages the Amazon Sidewalk protocol to connect to the cloud using a Bluetooth connection. The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. The user interacts with the endpoint either by pressing the main board buttons or issuing CLI commands.

> **Ⓘ INFO Ⓘ**: This application can be used with the KG100S module or Sidewalk-supported EFR32 series 2 SoCs with at least 512kB flash size.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. In order to perform these tasks and procure access to a Sidewalk gateway, complete the initial software and hardware setup steps described in [Getting Started: Prerequisites](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites).

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

## Interacting with the Endpoint

Send commands to the endpoint using either main board button presses or CLI commands.
The J-Link RTT interface provides access to the CLI commands. The following table shows the available commands and their effect, with equivalent button presses where applicable.

| Command | Description | Example | Main Board Button |
|---|---|---|---|
| connect | Connects the endpoint to a Sidewalk gateway | > connect | PB0/BTN0 |
| update | Sends an updated counter value to the cloud | > update | PB1/BTN1 |
| send | Sends an ASCII string of length of N to the cloud | > send N | N/A |
| reset | Deregisters the Sidewalk device and restores settings to factory defaults. | > reset | N/A |

> **⚠ WARNING ⚠**: The `send` command may crash if no argument or too small, large argument is provided. Size (N) must be in the valid range of [1, 20].

> **⚠ WARNING ⚠**: The `reset` command is used to unregister your device with the cloud. It can only be called on a registered AND time synced device.

## Interacting with the Cloud

Gain additional insight on network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
