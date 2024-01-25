# Amazon Sidewalk - SoC Dynamic Multiprotocol Light

This is a Dynamic Multiprotocol reference application demonstrating a light bulb that can be switched via Bluetooth or Amazon Sidewalk FSK.

It allows a BLE central device to control the LED on the mainboard and receive button press notifications. To test this demo, install EFR Connect mobile application. Simultaneously this sample application leverages the Amazon Sidewalk protocol to connect to the cloud using sub-GHz FSK modulation. The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud.

The user interacts with the endpoint either by pressing the main board buttons, through the BLE EFR Connect application or through the AWS cloud by issuing CLI commands.

You can learn more about Silicon Labs Multiprotocol libraries on [Silicon Labs website](https://www.silabs.com/wireless/multiprotocol?tab=learn).

> **Ⓘ INFO Ⓘ**: This application can be used with Sidewalk-supported EFR32 series 2 xG28 SoC with at least 1024kB flash size.

> **⚠ WARNING ⚠**: Sub-GHz communication occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. To perform these tasks and procure access to a Sidewalk gateway, complete the initial software and hardware setup steps described in [Getting Started: Prerequisites](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites).

> **Ⓘ INFO Ⓘ**: Make note of the additional sub-GHz considerations discussed in the [Silicon Labs Wireless Development Kit](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites#silicon-labs-wireless-development-kit) section of the hardware prequisites.

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Prepare the Cloud and Endpoint

Create AWS resources to interface with your endpoint and couple the application image with device-specific credentials by following the steps in [Getting Started: Provision your Amazon Sidewalk Device](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/provision-your-device).

## Modulation Control

The Dynamic Multiprotocol application is supported only on Sidewalk FSK and regular BLE radio layer.

## Interacting with the Endpoint

Send commands to the endpoint using either the main board button presses or CLI commands. The J-Link RTT interface provides access to the CLI commands. The following table shows the available commands and their effect, with equivalent button presses where applicable. Some commands are dependent on the currently selected radio layer.

If you press the BTN0, the LED0 should turn on. This will send a notification message to both the BLE and Sidewalk cloud applications. You can also control the LED from both the BLE and Sidewalk cloud applications. Details can be found below in the according section.

LED1 is used to represent BLE state, on for BLE started and off for stopped.

### Radio board CLI and push-button actions

| Command | Description | Example | Main Board Button |
|---|---|---|---|
| help | Display help menu | > help | N/A |
| toggle_led | Toggle LED | > toggle_led 0 | PB0/BTN0 short press |
| ble_start_stop | Switch BLE start/stop | > ble_start_stop | PB0/BTN0 long press |
| send | Sends an updated counter value to the cloud | > send | PB1/BTN1 |
| reset | Performs software reset (1) or unregisters the Sidewalk Endpoint (2) | > reset 2 | N/A |

> **⚠ WARNING ⚠**: The `reset 2` command is used to unregister your device with the cloud. It can only be called on a registered AND time synced device.

### BLE interaction

You'll need to have the EFR32 Connect mobile app. Once installed you can go to the demo view and select `Blinky`. Then you should see `Blinky Example` appear in the device list. If not make sure that the application is running and not in a faulty state (e.g. sidewalk stack couldn't start).

Once connected you should see a bulb and an button. For this demo the behavior of the `Blinky` interface is not the same as advertised :

- Pressing the bulb will toggle the LED but it doesn't reflect the state of the LED itself
- The status of the LED is reflected by the button displayed below the bulb

#### Toggle the LED

Press the bulb to toggle the LED state. Note that the led state is not in sync with the bulb visual, as the `Blinky` demo is not designed for that.

#### State of the LED

You can see the LED state reported by the button visual below the bulb.

### Sidewalk interaction

Once your Sidewalk configuration is up and running you can connect to the MQTT test client to see the notification of the LED updates. You can issue a command from the cloud to the endpoint to toggle the LED using Sidewalk FSK.

You can refer to the [Interacting with the cloud documentation](https://docs.silabs.com/amazon-sidewalk/1.0.0/sidewalk-getting-started/interacting-with-the-cloud) for instructions.

#### Toggle the LED

To toggle the LED via Sidewalk FSK, send the following AWS CLI command:

`aws iotwireless send-data-to-wireless-device --id="Wireless-Device-ID" --transmit-mode 0 --payload-data="AQUQAQA=" --wireless-metadata "Sidewalk={Seq=1}"`

> **Ⓘ INFO Ⓘ**: Don't forget to specify your Wireless device ID and increment/change the Seq ID (`--wireless-metadata "Sidewalk={Seq=1}"`) for each message.

#### State of the LED

In MQTT client you can subscribe to the `#` channel to see all incoming messages. A specific message is sent from the endpoint to indicate the state of the LED.

## Interacting with the Cloud

Gain additional insight on the network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
