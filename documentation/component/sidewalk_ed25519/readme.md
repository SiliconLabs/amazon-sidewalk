# Amazon Sidewalk - Ed25519 Cryptography Component

The Sidewalk Ed25519 component provides software-based cryptographic signature and verification capabilities for Amazon Sidewalk applications. This component implements the Ed25519 digital signature algorithm for secure communication and authentication, specifically designed for devices that lack hardware support for ed25519 cryptography.

## Component Overview

The `Sidewalk Ed25519` component provides:

- **Digital Signatures**: Ed25519 signature generation and verification
- **Cryptographic Security**: High-security elliptic curve cryptography
- **Key Management**: Secure key generation and handling
- **Authentication**: Device and message authentication
- **Software Implementation**: Pure software cryptography for devices without hardware support

## Hardware Requirements

### Supported Hardware
- **EFR32xG27 Devices**: Primary target - devices without hardware ed25519 support
- **Other EFR32 Devices**: Any EFR32 device with sufficient memory
- **Memory Requirements**: Adequate RAM and flash for cryptographic operations
- **No External Hardware**: Uses software-based cryptography

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk Ed25519` in your project


## Additional Resources

To use the cryptographic operations, see the [Security and Crypto APIs](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#security-and-crypto-apis) in the Sidewalk SDK API documentation.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **MIT License**: Open source cryptography implementation

See the component source files for complete license information. 