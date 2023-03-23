# Amazon Sidewalk - SoC Bluetooth Sub-GHz Out-of-the-Box (OOB) Demo

> **Ⓘ INFO Ⓘ**: This application image is provided for the sole purpose of restoring the factory-default OOB demo application on KG100S Sidewalk Module Radio Boards (BRD4332A) included in the [Silicon Labs Pro Kit for Amazon Sidewalk](https://www.silabs.com/development-tools/wireless/proprietary/amazon-sidewalk-pro-kit). It will not function on any other device, nor on any BRD4332A boards sourced independently from the above kits.

The OOB (Bluetooth & sub-GHz) sample application leverages the Amazon Sidewalk protocol to connect to the cloud using sub-GHz FSK / CSS modulation (after an initial registration phase over BLE, if necessary). The Sidewalk endpoint connects to a gateway, allowing it to exchange data with the AWS cloud. The user interacts with the endpoint either by pressing the main board buttons or through GUI elements in the associated web-based application running in AWS.

> **⚠ WARNING ⚠**: Sub-GHz communication occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

## Prerequisites

This OOB demonstration relies on AWS resources in the cloud that have already been deployed to support the application. No software prerequisites are required to run the OOB demo, though you must first [Prepare the Kit Hardware](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-demo/prepare-kit-hardware) and [Ensure Access to the Amazon Sidewalk Network](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-demo/network-availability).

## Flashing the Demo Firmware Image

If the KG100S radio board has been erased, the original demo functionality can be restored by flashing the pre-compiled *Amazon Sidewalk - SoC Bluetooth sub-GHz OOB* application image to the device.

> **⚠ WARNING ⚠**: The OOB demo relies on device-specific credentials pre-flashed to the USERDATA page on your device. This page is not affected by masserase operations, but can be explicitly erased by a targeted page erase. [Credentials Backup/Restore Feature](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-developers-guide/application-development#credentials-backup-restore-feature) describes a process by which these credentials are used to restore a working out-of-the-box demo application after a masserase. However, care should be taken to NOT perform a page erase of USERDATA, or the OOB demo cannot be restored.

To recover the demo application, perform the following tasks in Simplicity Studio:

- Connect the main board (with KG100S radio board mounted) to your computer.
- A new entry should appear in the **Launcher** perspective's **Debug Adapters** view.
- Select the board and make sure your Gecko SDK with Amazon Sidewalk SDK extention installed is selected on the **General Information** card.
- Go to the **EXAMPLE PROJECTS & DEMOS** tab.
- Filter the example list by typing **_sidewalk_** in the "**Filter on Keywords**" field and hit enter.
- Click **Run** next to the **Amazon Sidewalk - SoC Bluetooth sub-GHz OOB** demo.
- In the **Device Selection** window that appears, select your KG100S device in the list of connected devices, then click **OK**.

The OOB demo will now be flashed to your device, restoring the original functionality if the USERDATA resources are still intact.

## Running the Demo

Quick start instructions:

1) Install radio board (KG100S) onto the main board (BRD4002)
2) Connect main board to your host PC (or any USB power source)
3) Wait for LED1 on the main board to stabilize
4) Read QR code sticker (applied to the KG100S) with mobile device, open the embedded URL in a browser
5) Interact with main board and demo AWS application, demonstrating uplink/downlink over Amazon Sidewalk between the end device and the cloud

See [Running the Out-of-the-Box Demo](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-demo/run-the-demo) for a full description of how to use the OOB demo.

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
