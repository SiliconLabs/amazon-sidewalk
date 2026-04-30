# Amazon Sidewalk - KG100S Module Support Component

This is the board support component for the Quectel KG100S module. The KG100S is a compact module featuring an EFR32BG21 microcontroller with an integrated Semtech SX1262 Sub-GHz radio, enabling both BLE and Sub-GHz communication. It defines the required pin configurations, SPI routing, and hardware initialization necessary for the Sidewalk SDK to operate correctly on custom hardware designs based on KG100S. It should be included when targeting KG100S module from a EFR32BG21 host MCUs project.

> **⚠ WARNING ⚠**: When using the KG100S module, always add the `Sidewalk KG100S Support` component to your project, NOT the `Sidewalk Sub-GHz SX1262` component directly. The KG100S component will automatically include the SX1262 driver with the proper configuration for the module.

> **⚠ WARNING ⚠**: Sub-GHz communication operates in the 915MHz band (US only). Amazon Sidewalk is currently not supported in Europe.

## Component Overview

The `Sidewalk KG100S Support` component provides:

- **Complete Module Support**: Hardware support for KG100S module
- **Integrated SX1262 Radio**: Built-in Sub-GHz radio with automatic configuration
- **Dual Connectivity**: BLE and Sub-GHz communication capabilities
- **Pre-configured Setup**: Automatic GPIO and SPI configuration

## Hardware Requirements

### KG100S Module

- EFR32BG21 microcontroller with BLE radio
- Integrated Semtech SX1262 Sub-GHz radio (902-928 MHz, up to 20 dBm)
- Can be used on custom hardware or with BRD4332A development board

> **Ⓘ INFO Ⓘ**: This component is specifically designed for the KG100S module and can be used on any hardware platform that includes this module.

## Usage

### Integration Steps

1. Add `Sidewalk KG100S Support` component in Simplicity Studio
2. Ensure your hardware includes the KG100S module
3. Configure your Sidewalk application
4. Build and flash the module

The component automatically includes the SX1262 radio driver and configures all necessary interfaces.

### Protocol Configuration

The KG100S component is designed for BLE + Sub-GHz operation only:
- **Standard Mode**: The component automatically includes the `sidewalk_subghz_sx1262` driver
- **DMP Mode** (optional): Add `sidewalk_dmp_ble_support` component for dynamic multiprotocol operation. In DMP mode, the SX1262 component is not included.

> **Ⓘ INFO Ⓘ**: The component configures GPIO0 as push-pull output to control Tx/Rx states on the SX1262 radio.

## Compatibility

### Component Conflicts

This component cannot be used with:
- `sidewalk_subghz_efr32xgxx`
- `sidewalk_subghz_lr1110`

### Device Support

- **Supported**: KG100S module on any hardware platform (custom boards or BRD4332A development board)
- **Not Supported**: Other EFR32 devices or modules

## Report Bugs & Get Support

For technical support or bug reports, visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license
- **Amazon Sidewalk License**: Additional terms for Amazon Sidewalk functionality

See the component source files for complete license information.
