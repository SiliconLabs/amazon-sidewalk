# Amazon Sidewalk - Log Stack RTT Component

The Sidewalk Log Stack RTT component provides RTT (Real-Time Transfer) logging functionality for Amazon Sidewalk stack operations. This component enables stack-level logging output through RTT interface for debugging and monitoring.

## Component Overview

The `Sidewalk Log Stack RTT` component provides:

- **RTT Logging**: Real-time logging output through RTT interface
- **Stack Logging**: Sidewalk stack log messages
- **Debug Support**: Debug information output for stack operations
- **Real-Time Monitoring**: Live log monitoring capabilities
- **Independent Configuration**: Can be configured independently from app and PAL logging

## Hardware Requirements

### Supported Hardware
- **EFR32 Devices**: Any EFR32 device with RTT support
- **RTT Interface**: RTT communication interface
- **Debug Connection**: J-Link or compatible debug interface

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk Log Stack RTT` in your project
2. **Configure RTT**: Ensure RTT interface is properly configured
3. **Connect Debugger**: Connect J-Link or compatible debug interface
4. **Monitor Logs**: Use RTT viewer to monitor stack log output

### Logging Interface Selection

**Important**: RTT and VCOM logging interfaces cannot be used simultaneously. However, each layer can use a different interface, but within each layer, only one interface type can be active at a time.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 