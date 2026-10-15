# Amazon Sidewalk - DMP BLE Support Component

The Sidewalk DMP BLE Support component provides Dynamic Multiprotocol (DMP) configuration for BLE communication in Amazon Sidewalk applications. Enabling BLE to be used for both Sidewalk communication and custom usage simultaneously.

## Component Overview

The `Sidewalk DMP BLE Support` component provides:

- **Configuration Management**: Ensures RAIL multiprotocol is included in the project
- **Component Validation**: Prevents Sub-GHz components from being included
- **DMP BLE Support**: Dynamic Multiprotocol for BLE communication
- **Dual Usage**: BLE can be used for both Sidewalk and custom communication
- **User Implementation**: Most application-level implementation is user responsibility

## Hardware Requirements

### Supported Hardware
- **EFR32 Devices**: Any EFR32 device with BLE support
- **BLE Radio**: Built-in BLE radio capability
- **No External Hardware**: Uses internal BLE radio

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk DMP BLE Support` in your project
2. **Configure BLE**: Ensure BLE radio is properly configured
3. **Enable DMP**: Configure DMP functionality for BLE
4. **Configure Dual Usage**: Set up both Sidewalk and custom BLE communication
5. **Test Management**: Verify DMP BLE functionality

### Dual Communication Support

This component enables the BLE radio to handle both:
- **Sidewalk Communication**: Standard Amazon Sidewalk protocol communication
- **Custom BLE Usage**: Custom BLE services and applications

Both communication types can operate simultaneously, allowing developers to implement custom BLE functionality while maintaining Sidewalk connectivity.

### Implementation Responsibility

**Important**: This component primarily handles configuration and validation. Most of the application-level implementation for custom BLE functionality is the responsibility of the user. The component ensures:
- RAIL multiprotocol is properly included
- No Sub-GHz components are present in the project

## Device Compatibility

**Supported**: EFR32 devices with BLE radio

**Not Supported**: 
- Devices without BLE radio
- Non-EFR32 devices

**Requires**: EFR32 device with BLE radio

## Compatibility

### Works With
- All Amazon Sidewalk components
- BLE radio components

### Conflicts With
- **Sub-GHz Components**: BLE DMP does not work together with any Sub-GHz components
- **Sub-GHz Radio**: Cannot be used with SX1262, LR1110, or EFR32xGxx Sub-GHz components

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 