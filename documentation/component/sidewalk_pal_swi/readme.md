# Amazon Sidewalk - PAL SWI Component

The Sidewalk PAL SWI component provides software interrupt (SWI) functionality for Amazon Sidewalk applications. This component implements platform abstraction layer functions for software interrupt handling and management.

## Component Overview

The `Sidewalk PAL SWI` component provides:

- **Software Interrupts**: SWI handling and management
- **Platform Abstraction**: Hardware-independent SWI implementation
- **Interrupt Management**: Software interrupt scheduling and execution
- **FreeRTOS Integration**: RTOS-aware interrupt handling
- **Dual Mode Support**: FreeRTOS thread-based and hardware-based SWI options

## Usage

### Integration Steps

1. **Add Component**: Include `Sidewalk PAL SWI` in your project
2. **Configure FreeRTOS**: Ensure FreeRTOS is properly configured
3. **Initialize SWI**: Call initialization function during startup
4. **Test Interrupts**: Verify SWI functionality

### Software Interrupt Modes

The component supports two software interrupt implementation modes:

#### FreeRTOS Thread Mode
- **Implementation**: Uses a dedicated FreeRTOS thread called "SWI" to handle software interrupts
- **Context**: Event callbacks are executed from task context
- **Advantages**: Safe for complex operations, can use FreeRTOS APIs
- **Use Case**: When interrupt handlers need to perform complex operations or use RTOS services

#### Hardware-Based Mode
- **Implementation**: Uses Silicon Labs' hardware-based software interrupt mechanism
- **Context**: Event callbacks are executed from interrupt context
- **Advantages**: Lower latency, more efficient for simple operations
- **Use Case**: When fast response time is critical and operations are simple

## Additional Resources

To use this component through SWI Interface, see the [SWI Interface](../../suds/sld583-sidewalk-services-api/sidewalk-sdk-api.md#swi-interface) in the Sidewalk SDK API documentation.

## Report Bugs & Get Support

For technical support, bug reports, or questions about this component, please visit the [Silicon Labs Community](https://community.silabs.com).

## License

This component is licensed under:
- **Zlib License**: Silicon Labs software license

See the component source files for complete license information. 