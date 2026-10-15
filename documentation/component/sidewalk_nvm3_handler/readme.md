# Amazon Sidewalk - NVM3 Handler Component

The Sidewalk NVM3 Handler component provides non-volatile memory management for Amazon Sidewalk applications. This component handles persistent storage of application data using the same NVM3 region as other components but ensures correct key ID allocation (0xA0000 - 0xA1FFF) for application data storage.

## Component Overview

The `Sidewalk NVM3 Handler` component provides:

- **Application Data Storage**: Non-volatile memory management for application data
- **Key ID Management**: Ensures correct NVM3 key ID usage (0xA0000 - 0xA1FFF)
- **State Management**: Application state persistence across reboots
- **Data Persistence**: Reliable storage for user application data
- **Memory Management**: Efficient flash memory utilization for app data

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk NVM3 Handler` in your project
2. **Configure NVM3**: Ensure NVM3 library is properly configured
3. **Initialize Handler**: Call initialization function during startup
4. **Store Application Data**: Use the handler for your application data only

### Key ID Requirements
When using this component for application data storage, ensure that your NVM3 keys are within the application key id range (0xA0000 - 0xA1FFF). Keys outside this range may conflict with other components using the same NVM3 region.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 