# Amazon Sidewalk - BLE Radio Component

The Sidewalk BLE component provides Bluetooth Low Energy (BLE) communication support for Amazon Sidewalk applications. This component enables BLE connectivity for device registration and data exchange with Amazon Sidewalk gateways.

## Component Overview

The `Sidewalk BLE` component provides:

- **BLE Communication**: Bluetooth Low Energy connectivity
- **Device Registration**: BLE-based device registration with Amazon Sidewalk
- **Data Exchange**: BLE communication for data transmission
- **Gateway Connection**: Connection to Amazon Sidewalk gateways

## Hardware Requirements

### Supported Hardware
- **EFR32 Devices**: Any EFR32 device with built-in BLE radio
- **No External Hardware**: Uses the microcontroller's built-in BLE radio
- **Antenna**: Built-in antenna or external antenna connection

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk BLE` in your project
2. **Configure Device**: Ensure your project targets a BLE-capable EFR32 device
3. **Test Communication**: Verify BLE advertisement and connection

## Device Compatibility

**Supported**: EFR32 devices with built-in BLE radio

**Not Supported**: 
- Devices without BLE radio
- Non-EFR32 devices

## Compatibility

### Works With
- All Sub-GHz components (SX1262, LR1110, EFR32xGxx)

## Additional Resources

For detailed information about BLE adaptation and implementation in Amazon Sidewalk, see the [BLE Adaptation](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#ble-adaptation) in the Sidewalk SDK API documentation.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license
- **Amazon Sidewalk License**: Additional terms for Amazon Sidewalk functionality

See the component source files for complete license information. 