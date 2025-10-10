# Amazon Sidewalk - Log Stack VCOM Component

The Sidewalk Log Stack VCOM component provides VCOM (Virtual COM Port) logging functionality for Amazon Sidewalk stack operations. This component enables stack-level logging output through virtual serial port for debugging and monitoring.

## Component Overview

The `Sidewalk Log Stack VCOM` component provides:

- **VCOM Logging**: Virtual COM port logging output
- **Stack Logging**: Sidewalk stack log messages
- **Serial Output**: Serial port logging interface for stack operations
- **USB Connection**: USB-based virtual serial port
- **Independent Configuration**: Can be configured independently from app and PAL logging

## Hardware Requirements

### Supported Hardware
- **EFR32 Devices**: Any EFR32 device with USB support
- **USB Interface**: USB communication interface
- **VCOM Driver**: Virtual COM port driver support

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk Log Stack VCOM` in your project
2. **Configure USB**: Ensure USB interface is properly configured
3. **Install VCOM Driver**: Install virtual COM port driver
4. **Monitor Logs**: Use serial terminal to monitor stack log output

### Logging Interface Selection

**Important**: RTT and VCOM logging interfaces cannot be used simultaneously. However, each layer can use a different interface, but within each layer, only one interface type can be active at a time.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 