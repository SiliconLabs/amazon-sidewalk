# Amazon Sidewalk - SoC Production Device Provisioner (PDP)

The Production Device Provisioner application is a tool to enable your production product to leverage Secure Vault, generating the key pair directly on the device thus, limiting the exposure of the private key.

In Sidewalk, you can leverage Secure Vault to store sensitive data (private keys) in a secure place. A set of scripts and this application is provided to use the Secure Element in the Amazon Sidewalk context.

The PDP application is used to exchange certificate data and to communicate with the Secure Element through APIs. The PDP application can be used for provisioning and is automatically deleted upon reboot, as it is a transient application running in RAM.

For more information on product manufacturing in Sidewalk context, you can take a look at our documentation [Manufacture a product](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-manufacture-product/).

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Use PDP in Production

For a detailed walkthrough on how to use this application in production, you can check our documentation [Manufacture a product: On-Device Certificate Generation](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-manufacture-product/#on-device-certificate-generation)

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you find to us via [Silicon Labs Community](https://community.silabs.com).
