# Amazon Sidewalk - SoC Bluetooth Sub-GHz CLI Application

The Bluetooth sub-GHz Command Line Interface (CLI) sample application allows the user to interact with the endpoint using CLI commands. The application leverages the Amazon Sidewalk protocol to exchange data between the endpoint and the AWS Cloud using one of the 3 radio layers. It is possible to initialize and start the stack using any one of the 3 radio layers (BLE, FSK, or CSS). A one-time registration phase (using either BLE or FSK, as registration does not occur over CSS) is required at first boot.

> **Ⓘ INFO Ⓘ**: This application can be used with the KG100S module or Sidewalk-supported EFR32 series 2 SoCs with at least 768kB flash size.

> **⚠ WARNING ⚠**: Sub-GHz communication (FSK and CSS) occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

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

## Interacting with the Endpoint through the CLI

Control the endpoint's behavior and configuration using the CLI commands described in the following sections. The J-Link RTT interface provides access to the CLI commands.

> **Ⓘ INFO Ⓘ**: When running this application for the first time on a new device, the endpoint will automatically perform a one-time [Touchless Registration](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-protocol-overview/frustration-free-network) with Amazon Sidewalk. This only occurs over BLE or FSK (not CSS), so at first boot you must start one of these two communication links to initiate registration. Registration is a persistent state, so from that point forward any of the 3 links can be selected at will.

The CLI is divided into two sets of commands:
- Commands starting with `sidewalk` or `sid` are used to control the Sidewalk stack.
- Commands starting with `attributes` or `att` are used to control the stack settings.

You can display the available commands by entering `help` in the console.

### Sidewalk Commands

To see the available commands, enter the following command in the console:

`sid help`

The list of available commands is output on the console with the associated help. Following is an extended description of and examples of how to use each command.

| Command | Description | Example |
|---|---|---|
| sid init \<link\> | Initialize Sidewalk stack for the selected communication link (fsk/css/ble). This function can only be called once. You have to use deinit to call it again. | > sid init fsk |
| sid start \<link\> | Start Sidewalk stack for the selected communication link (fsk/css/ble). | > sid start fsk |
| sid stop \<link\> | Stop Sidewalk stack for the selected communication link (fsk/css/ble). | > sid stop fsk |
| sid deinit | Deinitialize Sidewalk stack. | > sid deinit |
| sid reset | Deregisters the Sidewalk device and restores settings to factory defaults. | > sid reset |
| sid bleconnect | Initiate BLE connection request. | > sid bleconnect |
| sid send \<message_type\> \<payload\> | Send a custom message to the cloud (Message types get/set/notify/response). | > sid send notify "my custom payload" |

To start and switch between links, the command sequence is as follows:

1. `sid init <your link>`
2. `sid start <your link>`
3. `sid stop <your link>`
4. `sid deinit`

Only one link can be initialized and started at any time. You have to stop and deinitialize the current link before you can start a new one.

The `reset` command is used to unregister your device with the cloud. It can only be called on a registered AND time synced device. To unregister, the device needs to send a message to the cloud, so it needs to be synchronized with AWS.

### Sidewalk Settings

To see the available settings commands, enter the following command in the console:

`att help`

The command-line interface maintains several settings. The Sidewalk settings are distributed in five sections: *device*, *radio*, *ble*, *fsk* and *css*. They can be listed by entering:

`att get <section name>`

The Sidewalk settings are listed with their current state/value. Some of them can be modified by using the following command prototype:

`att set <section name>.<setting name> <value>`

It is also possible to save your configuration or reset the parameters to their default values with `att save` and `att reset`.

### *sidewalk* Section Settings

Settings in the *sidewalk* section are directly related to the Sidewalk general state. A detailed setting list can be found below.

| Variable | R/W | Type | Values | Description |
|---|---|---|---|---|
| sidewalk.started_link | R | string | BLE<br>FSK<br>CSS<br>NONE | Currently started link |
| sidewalk.region | R | string | US | Regulatory domain for Sidewalk |
| sidewalk.state | R | string | READY<br>NOT READY<br>SECURE CHANNEL READY | Device state |
| sidewalk.time | R | string | "1350808426.972536238" | Gives Endpoint time (GPS time format) |

### *radio* Section Settings

Settings in the *radio* section are directly related to the Sidewalk radio parameters. A detailed setting list can be found below.

| Variable | R | Type | Values | Description |
|---|---|---|---|---|
| radio.rssi | R | integer | Negative value, for example -30dBm | RSSI from last received message (in dBm) |
| radio.snr | R | integer | Numeric value, for example 17dB | SNR from last received message (in dB) |

> **⚠ WARNING ⚠**: RSSI and SNR values are not valid until at least one message has been received.

### *ble* Section Settings

The settings in the *ble* section are directly related to the Sidewalk BLE stack behavior. A detailed setting list can be found below.

| Variable | R/W | Type | Values | Description |
|---|---|---|---|---|
| ble.ble_mtu | R | integer | 256 | BLE Maximum Transmission Unit in bytes |
| ble.random_mac | R | string | PUBLIC<br>RANDOM_PRIVATE_NON_RESOLVABLE<br>STATIC_RANDOM<br>RANDOM_PRIVATE_RESOLVABLE | Randomize MAC address |
| ble.output_power | R | integer | Numeric value, for example 22 for 22dBm | TX output power in dBm |

### *fsk* Section Settings

Settings in the *fsk* section are directly related to the Sidewalk FSK stack behavior. A detailed setting list can be found below.

| Variable | R/W | Type | Values | Description |
|---|---|---|---|---|
| fsk.data_rate | R | integer | data rate value in Kbps | Current data rate used in FSK in Kbps |
| fsk.min_freq | R | integer | frequency value in MHz | Start frequency for FSK in MHz |
| fsk.max_freq | R | integer | frequency value in MHz | Stop frequency for FSK in MHz |
| fsk.power_profile | R/W | integer | 1<br>2 | Power profile currently in use |
| fsk.fsk_mtu | R | integer | 256 | FSK Maximum Transmission Unit in bytes |
| fsk.rx_window_count | R | integer | INFINITE | Number of RX opportunities (always infinite) |
| fsk.rx_window_separation | R/W | integer | 252 | Time between RX opportunities in ms |

### *css* Section Settings

Settings in the *css* section are directly related to the Sidewalk CSS stack behavior. A detailed setting list can be found below.

| Variable | R/W | Type | Values | Description |
|---|---|---|---|---|
| css.bandwidth | R | integer | bandwidth value in KHz | Current bandwidth used in CSS in kHz |
| css.min_freq | R | integer | frequency value in MHz | Start frequency for CSS in MHz |
| css.max_freq | R | integer | frequency value in MHz | Stop frequency for CSS in MHz |
| css.power_profile | R/W | string | A<br>B | Power profile currently in use |
| css.css_mtu | R | integer | 256 | CSS Maximum Transmission Unit in bytes |
| css.rx_window_count | R/W | integer | - | Choose the number of RX opportunities |
| css.rx_window_separation | R/W | integer | - | Time between RX opportunities |

> **⚠ WARNING ⚠**: Custom rx_window_separation is not yet supported - only the default 5 seconds is currently accepted.

## Interacting with the Cloud

Gain additional insight on network activity from the cloud perspective by using the techniques described in [Getting Started: Interacting with the Cloud](https://docs.silabs.com/amazon-sidewalk/latest/interacting-with-the-cloud).

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
