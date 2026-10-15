# Amazon Sidewalk - Log App RTT Component

The Sidewalk Log App RTT component provides RTT (Real-Time Transfer) logging functionality for Amazon Sidewalk applications. This component enables application-level logging output through RTT interface for debugging and monitoring.

## Component Overview

The `Sidewalk Log App RTT` component provides:

- **RTT Logging**: Real-time logging output through RTT interface
- **Application Logging**: Application-level log messages
- **Debug Support**: Debug information output
- **Real-Time Monitoring**: Live log monitoring capabilities
- **Independent Configuration**: Can be configured independently from PAL and stack logging

## Hardware Requirements

### Supported Hardware
- **EFR32 Devices**: Any EFR32 device with RTT support
- **RTT Interface**: RTT communication interface
- **Debug Connection**: J-Link or compatible debug interface

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk Log App RTT` in your project
2. **Configure RTT**: Ensure RTT interface is properly configured
3. **Connect Debugger**: Connect J-Link or compatible debug interface
4. **Monitor Logs**: Use RTT viewer to monitor log output

### Logging Interface Selection

**Important**: RTT and VCOM logging interfaces cannot be used simultaneously. However, each layer can use a different interface, but within each layer, only one interface type can be active at a time.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 