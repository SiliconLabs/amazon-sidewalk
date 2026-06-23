# Amazon Sidewalk - SoC Cloud Light

The Cloud Light sample application is a lightweight end-to-end solution that pairs a Sidewalk endpoint with a user-deployed AWS backend and a simple web UI. Because the cloud stack and the web app are deployed to your own AWS account, this example sits between the [Hello Neighbor](../amazon_sidewalk_soc_hello_neighbor/readme.md) example (a minimal endpoint without a companion web UI) and the [Out-of-the-Box (OOB) Demo](../amazon_sidewalk_soc_oob/bt/readme.md) (a full Silicon Labs hosted demo): you deploy the supplied cloud stack and web app yourself, register your own device, and then interact with it either through the main board buttons or through GUI elements in the web UI.

The application leverages the Amazon Sidewalk protocol to connect the endpoint to the cloud. The web UI displays the device's capabilities (button counters, LED states, current link, temperature, etc.) and lets you send downlink commands back to the endpoint.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. 

> **Ⓘ INFO Ⓘ**: Make note of the additional sub-GHz considerations discussed in the [Silicon Labs Wireless Development Kit](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites#silicon-labs-wireless-development-kit) section of the hardware prerequisites.

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Prepare the Cloud and Endpoint

Unlike the OOB demo, the Cloud Light example uses cloud resources that you deploy yourself:

1. Deploy the Cloud Light stack to your own AWS account by following [our documentation](https://github.com/SiliconLabsSoftware/amazon-sidewalk-cloud-light-sample-app). Save the URL given at the end of the deployment, it will be used to interact with the end device.
2. Generate a new manufacturing page to provision your device by leveraging the [Sidewalk Assistant End Device Tab](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/assistant-user-guide#end-device-tab). You need to set the destination name to `CloudLightDestination` for the device to connect to the newly deployed Web Application.

## Modulation Control

By default, the Cloud Light application starts on a single radio layer (BLE on boards that support it, otherwise FSK). Only one link is active at a time, but the application supports run-time link switching (BLE → FSK → CSS, depending on what the board supports), triggered either by a long button press or from the web UI.

> **⚠ WARNING ⚠**: Radio layers not supported by the radio board are skipped while switching. On boards that support a single radio layer only (e.g., the xG23 family), the link switch action is not available.

## Running the Application

Quick start instructions:

1. Power up the board and wait for the device to register and time-sync with Sidewalk.
2. Open the web UI:
    - **With display**: scan the QR code shown on the LCD with a mobile device.
    - **Without display**: open the base URL saved during cloud deployment in a browser, enter the password you configured, and select your device by SMSN.
3. Interact with the endpoint using the main board buttons or GUI elements in the web app. Endpoint events are reflected on the web UI as uplink messages, and downlink commands from the web UI act on the device (e.g., toggle LEDs, switch link).

### Push-button Actions

| Main Board Button | Action |
|---|---|
| BTN0 (short press) | Increments the button 0 short-press counter and reports it to the web UI. |
| BTN0 (long press) | Switches to the next supported link (BLE → FSK → CSS). |
| BTN1 (short press) | Increments the button 1 short-press counter and reports it to the web UI. |
| BTN1 (long press) | Toggles a boolean state and reports it to the web UI. On boards with BTN1 only, it switches the link instead. |

On boards equipped with a temperature sensor, the device also reports periodic temperature readings to the web UI, with the reporting interval shown as a separate capability.

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
