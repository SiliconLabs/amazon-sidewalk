# Amazon Sidewalk - DMP FSK Support Component

The Sidewalk DMP FSK Support component provides Dynamic Multiprotocol (DMP) functionality for FSK communication in Amazon Sidewalk applications. This component enables the integrated radio to be used for both FSK Sub-GHz communication and BLE simultaneously, and makes the EFR32xGxx PAL compatible with DMP operation.

## Component Overview

The `Sidewalk DMP FSK Support` component provides:

- **DMP FSK Support**: Dynamic Multiprotocol for FSK communication
- **Dual Radio Usage**: Integrated radio can be used for both FSK and BLE
- **PAL Compatibility**: Makes EFR32xGxx PAL compatible with DMP operation
- **Protocol Support**: DMP protocol implementation for FSK
- **FSK Integration**: DMP functionality integrated with FSK radio
- **Concurrent Operation**: Simultaneous FSK Sub-GHz and BLE communication

## Hardware Requirements

### Supported Hardware
- **EFR32 Devices**: Any EFR32 device with FSK radio support
- **Sub-GHz Radio**: Sub-GHz radio for FSK communication

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk DMP FSK Support` in your project
2. **Configure FSK**: Ensure FSK radio is properly configured
3. **Enable DMP**: Configure DMP functionality for FSK
4. **Configure Dual Radio**: Set up both FSK Sub-GHz and BLE communication
5. **Test Management**: Verify DMP FSK functionality

### Dual Radio Support

This component enables the integrated radio to handle both:
- **FSK Sub-GHz Communication**: FSK-based Sub-GHz radio communication
- **BLE Communication**: Bluetooth Low Energy communication

Both communication types can operate simultaneously, allowing developers to use both FSK Sub-GHz and BLE functionality on the same integrated radio.

### PAL DMP Compatibility

This component also ensures that the EFR32xGxx Platform Abstraction Layer (PAL) is properly configured for DMP operation, enabling the PAL to work correctly with dynamic multiprotocol functionality.

## Device Compatibility

**Supported**: 
- **EFR32xGxx Sub-GHz Component**: Only works with built-in EFR32xGxx Sub-GHz radio
- **EFR32 Devices**: EFR32 devices with integrated Sub-GHz radio

**Not Supported**: 
- **SX1262 External Radio**: Not compatible with SX1262 Sub-GHz component
- **LR1110 External Radio**: Not compatible with LR1110 Sub-GHz component
- **Devices without FSK radio**: Non-EFR32 devices

**Requires**: EFR32 device with integrated Sub-GHz radio (EFR32xGxx only)

## Compatibility

### Works With
- **EFR32xGxx Sub-GHz Component**: Built-in Sub-GHz radio component
- **BLE Components**: BLE radio components for dual radio usage
- **Other Amazon Sidewalk components**: All other Sidewalk components

### Conflicts With
- **SX1262 Sub-GHz Component**: Cannot be used with external SX1262 radio
- **LR1110 Sub-GHz Component**: Cannot be used with external LR1110 radio

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 