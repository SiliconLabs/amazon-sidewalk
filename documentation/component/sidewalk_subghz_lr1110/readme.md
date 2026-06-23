# Amazon Sidewalk - Sub-GHz LR1110 Radio Component

This component includes the driver necessary to operate the LR1110 transceiver for Amazon Sidewalk use cases, supporting FSK and CSS radio features. GNSS and Wi-Fi-based geolocation capabilities provided by LR1110 are not used or initialized in this integration. The driver is tailored to interact with the Sidewalk for compliant radio operations.

> **⚠ WARNING ⚠**: Sub-GHz communication operates in the 915MHz band, which is open in the US but may be restricted in other regions. Ensure compliance with local regulations.

> **⚠ WARNING ⚠**: Amazon Sidewalk is currently not supported in Europe.

## Component Overview

The `Sidewalk Sub-GHz LR1110` component provides:

- **Radio Driver**: Complete LR1110 radio driver implementation with HAL (Hardware Abstraction Layer)
- **Modulation Support**: FSK and CSS modulation schemes for Amazon Sidewalk
- **Regional Compliance**: Support for US915 frequency band
- **Power Management**: Configurable transmit power with regional limits
- **SPI Interface**: Standard SPI communication with the LR1110 chip
- **GPIO Control**: Reset, DIO, and busy signal management

## Hardware Requirements

### Supported Hardware Configurations

#### External LR1110 Board
- **External LR1110 Radio**: Semtech LR1110 Sub-GHz transceiver
- **SPI Interface**: Standard SPI communication with the external radio
- **GPIO Connections**: Using WSTK with development board and LR1110 extension board, the correct pins are configured by default

### Required Connections

#### GPIO Connections
| Signal | GPIO Pin | Description |
|--------|----------|-------------|
| `SL_PIN_NRESET` | Configurable | Radio reset signal (active low) |
| `SL_PIN_DIO` | Configurable | Radio interrupt signal |
| `SL_PIN_BUSY` | Configurable | Radio busy signal |
| `SL_PIN_NSS` | Configurable | SPI chip select |

## Features

### Radio Capabilities
- **Frequency Range**: 902-928 MHz (US915 band)
- **Transmit Power**: -9 to +22 dBm (hardware limited)
- **Regional Power Limits**:
  - US915: Maximum 20 dBm
- **Modulation Schemes**:
  - **FSK**: Frequency-Shift Keying for reliable data transmission
  - **CSS**: Chirp Spread Spectrum for long-range communication
- **Data Rates**: Configurable based on modulation and regional requirements

### Regional Support
The component supports the US915 regulatory region:

| Region | Frequency Band | Max TX Power | Antenna Gain |
|--------|----------------|--------------|--------------|
| US915 | 902-928 MHz | 20 dBm | 2.15 dBi |

> **Ⓘ INFO Ⓘ**: Amazon Sidewalk is currently only supported in the United States.

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk Sub-GHz LR1110` in your project
2. **Configure SPI**: Set up SPI peripheral in your project configuration
3. **Connect GPIO**: Wire the required GPIO signals to your LR1110 radio
4. **Configure Power**: Set transmit power based on regional limits
5. **Test Communication**: Verify SPI communication with the radio

> **Ⓘ INFO Ⓘ**: Steps 2 and 3 are performed automatically when using a mainboard with a development board and an LR1110 extension board.

### GPIO Configuration

The GPIOs required for the LR1110 radio (such as RESET, DIO, and BUSY) can be configured in your project at `config/app_gpio_config.h`. This file allows you to map the necessary GPIO pins to your hardware setup as needed.

### Transmit Power Configuration

#### Regional Power Limits
The component automatically enforces regional power limits:
- **US915**: Maximum 20 dBm transmit power
- Power is automatically limited to regional maximum

## Conflicts and Compatibility

### Component Conflicts
This component conflicts with other Sub-GHz implementations:
- `Sidewalk Sub-GHz SX1262` (Semtech SX1262)
- `Sidewalk Sub-GHz efr32xgxx` (EFR32 built-in radio for other families)

### Device Compatibility
- **Supported**: 
  - External LR1110 hardware with SPI interface
  - EFR32xG28 with external LR1110 board
- **Not Supported**: 
  - Built-in radio devices (EFR32xG23, EFR32xG25, except EFR32xG28 with external board)
  - Other external radio chips
  - LR1120 and LR1121 variants
  - KG100S (BRD4332A) module
- **Requires**: Hardware board with external radio support

> **Ⓘ INFO Ⓘ**: Only the LR1110 variant is supported. LR1120 and LR1121 variants are not supported in this component.

## Additional Resources

To use this component through Sub-GHz Interface, see the [Sub-GHz Interface](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#sub-ghz-interface) in the Sidewalk SDK API documentation.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license
- **Amazon Sidewalk License**: Additional terms for Amazon Sidewalk functionality

See the component source files for complete license information. 