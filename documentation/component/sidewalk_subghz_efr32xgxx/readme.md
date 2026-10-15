# Amazon Sidewalk - Sub-GHz EFR32xGxx Radio Component

This is the native Silicon Labs sub-GHz driver used for FSK-based Sidewalk communication on EFR32 platforms. It is tightly integrated with the RAIL library. The driver abstracts the PHY layer and enables Sub-GHz profile configuration for FSK mode in compliance with Sidewalk requirements. It can only be used with compatible parts like the xG23 or xG28.

> **⚠ WARNING ⚠**: Sub-GHz communication operates in the 915MHz band, which is open in the US but may be restricted in other regions. Ensure compliance with local regulations.

> **⚠ WARNING ⚠**: Amazon Sidewalk is currently not supported in Europe.

## Component Overview

The `Sidewalk Sub-GHz efr32xgxx` component provides:

- **Built-in Radio**: Native Sub-GHz radio support using Silicon Labs RAIL library
- **Modulation Support**: FSK modulation scheme for Amazon Sidewalk
- **Regional Compliance**: Support for US915 frequency band
- **Power Management**: Configurable transmit power with regional limits
- **Integrated Design**: No external radio hardware required
- **RAIL Library**: Leverages Silicon Labs RAIL (Radio Abstraction Interface Layer)

## Hardware Requirements

### Supported Hardware Configurations

#### Built-in EFR32xGxx Radio
- **EFR32xG23**: Built-in Sub-GHz radio with integrated PA
- **EFR32xG25**: Built-in Sub-GHz radio with integrated PA
- **EFR32xG28**: Built-in Sub-GHz radio with integrated PA
- **No External Hardware**: Uses the microcontroller's built-in radio
- **Integrated Power Amplifier**: Built-in PA with configurable settings

### Required Connections

#### Antenna Connection
- **Antenna**: Connect appropriate antenna for 900MHz operation
- **No External Radio**: All radio functionality is integrated into the microcontroller

## Features

### Radio Capabilities
- **Frequency Range**: 902-928 MHz (US915 band)
- **Transmit Power**: Configurable based on device capabilities
- **Regional Power Limits**:
  - US915: Maximum 20 dBm
- **Modulation Schemes**:
  - **FSK**: Frequency-Shift Keying for reliable data transmission
- **Data Rates**: Configurable based on modulation and regional requirements

### Integrated Features
- **Built-in PA**: Integrated power amplifier with configurable settings
- **RAIL Library**: Silicon Labs Radio Abstraction Interface Layer
- **Low Power**: Optimized for battery-powered applications
- **No External Components**: Complete radio solution in a single chip

### Regional Support
The component supports the US915 regulatory region:

| Region | Frequency Band | Max TX Power | Antenna Gain |
|--------|----------------|--------------|--------------|
| US915 | 902-928 MHz | 20 dBm | 2.15 dBi |

> **Ⓘ INFO Ⓘ**: Amazon Sidewalk is currently only supported in the United States.

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk Sub-GHz efr32xgxx` in your project
2. **Configure Device**: Ensure your project targets a supported EFR32xGxx device
3. **Connect Antenna**: Connect appropriate antenna for 900MHz operation
4. **Configure Power**: Set transmit power based on regional limits
5. **Test Communication**: Verify radio communication

### Transmit Power Configuration

#### Regional Power Limits
The component automatically enforces regional power limits:
- **US915**: Maximum 20 dBm transmit power
- Power is automatically limited to regional maximum

## Conflicts and Compatibility

### Component Conflicts
This component conflicts with other Sub-GHz implementations:
- `Sidewalk Sub-GHz SX1262` (Semtech SX1262)
- `Sidewalk Sub-GHz LR1110` (Semtech LR1110)

### Device Compatibility
- **Supported**: 
  - EFR32xG23 with built-in Sub-GHz radio
  - EFR32xG25 with built-in Sub-GHz radio
  - EFR32xG28 with built-in Sub-GHz radio
- **Not Supported**: 
  - EFR32xG21, EFR32xG24, EFR32xG26, EFR32xG27, EFR32xG29 (no built-in Sub-GHz radio)
  - External radio chips (SX1262, LR1110)
  - KG100S (BRD4332A) module
- **Requires**: EFR32xGxx device with built-in Sub-GHz radio

## Additional Resources

To use this component through Sub-GHz Interface, see the [Sub-GHz Interface](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#sub-ghz-interface) in the Sidewalk SDK API documentation.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license
- **Amazon Sidewalk License**: Additional terms for Amazon Sidewalk functionality

See the component source files for complete license information. 