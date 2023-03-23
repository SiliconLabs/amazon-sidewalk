# Amazon Sidewalk - SoC Bluetooth Out-of-the-Box (OOB) Demo

> **Ⓘ INFO Ⓘ**: This application image is provided for the sole purpose of restoring the factory-default OOB demo application on EFR32xG24 2.4 GHz 20 dBm Radio Boards (BRD4187C) included in the [Silicon Labs Pro Kit for Amazon Sidewalk](https://www.silabs.com/development-tools/wireless/proprietary/amazon-sidewalk-pro-kit). It will not function on any other device, nor on any BRD4187C boards sourced independently from the above kits.

The OOB (Bluetooth) sample application leverages the Amazon Sidewalk protocol to connect to the cloud using a Bluetooth connection. The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. The user interacts with the endpoint either by pressing the main board buttons or through GUI elements in the associated web-based application running in AWS.

## Prerequisites

This OOB demonstration relies on AWS resources in the cloud that have already been deployed to support the application. No software prerequisites are required to run the OOB demo, though you must first [Prepare the Kit Hardware](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-demo/prepare-kit-hardware) and [Ensure Access to the Amazon Sidewalk Network](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-demo/network-availability).

## Flashing the Demo Firmware Image

If the EFR32xG24 radio board has been erased, the original demo functionality can be restored by flashing the pre-compiled *Amazon Sidewalk - SoC Bluetooth OOB Demo* application image to the device.

> **⚠ WARNING ⚠**: The OOB demo relies on device-specific credentials pre-flashed to the USERDATA page on your device. This page is not affected by masserase operations, but can be explicitly erased by a targeted page erase. [Credentials Backup/Restore Feature](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-developers-guide/application-development#credentials-backup-restore-feature) describes a process by which these credentials are used to restore a working out-of-the-box demo application after a masserase. However, care should be taken to NOT perform a page erase of USERDATA, or the OOB demo cannot be restored.

To recover the demo application, perform the following tasks in Simplicity Studio:

- Connect the main board (with EFR32xG24 radio board mounted) to your computer.
- A new entry should appear in the **Launcher** perspective's **Debug Adapters** view.
- Select the board and make sure your Gecko SDK with Amazon Sidewalk SDK extention installed is selected on the **General Information** card.
- Go to the **EXAMPLE PROJECTS & DEMOS** tab.
- Filter the example list by typing **_sidewalk_** in the "**Filter on Keywords**" field and hit enter.
- Click **Run** next to the **Amazon Sidewalk - SoC Bluetooth OOB** demo.
- In the **Device Selection** window that appears, select your EFR32xG24 device in the list of connected devices, then click **OK**.

The OOB demo will now be flashed to your device, restoring the original functionality if the USERDATA resources are still intact.

## Running the Demo

Quick start instructions:

1) Install radio board (EFR32xG24) onto the main board (BRD4002)
2) Connect main board to your host PC (or any USB power source)
3) Wait for QR code to appear on the main board LCD
4) Read QR code with mobile device, open the embedded URL in a browser
5) Interact with main board and demo AWS application, demonstrating uplink/downlink over Amazon Sidewalk between the end device and the cloud

See [Running the Out-of-the-Box Demo](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-demo/run-the-demo) for a full description of how to use the OOB demo.

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
