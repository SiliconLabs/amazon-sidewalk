# Overview {#mainpage}

## PAL API Reference

The Sidewalk Platform Abstraction Layer (PAL) API provides a comprehensive set of interfaces and utilities for developing applications that leverage the Amazon Sidewalk network. This documentation covers various modules and components of the SDK, detailing their functionalities, key features, and usage guidelines. The API is organized into several groups, each focusing on specific aspects of the SDK, such as BLE adaptation, security, storage, and peripheral interfaces. By following this reference, developers can effectively integrate and utilize the Sidewalk SDK in their projects.

@defgroup sid_ifc Interfaces

## Interfaces

The Interfaces module provides a set of standardized interfaces that facilitate communication and interaction between different components of the Sidewalk SDK. These interfaces define the methods and structures required for various functionalities, ensuring consistency and interoperability across the SDK. By customizing interfaces, developers can better fit their needs according to their use case or the specifics of their custom hardware.

@defgroup sid_pal_ifc SAL

## Sidewalk Abstraction Layer

The Sidewalk Abstraction Layer (SAL) provides interfaces, such as cryptography, storage, timers, and peripheral interfaces.

@defgroup sid_pal_crypto_ifc_support Security and Crypto

The Security and Crypto module provides interfaces for implementing cryptographic operations within the Sidewalk SDK. These interfaces ensure secure data transmission and storage by offering functionalities such as encryption, decryption, hashing, and key management, enabling developers to protect sensitive information across different hardware platforms.

@defgroup sid_pal_common_ifc Common Interface

Provides common and generic platform interface code not specific to one PAL module.

@defgroup sid_pal_swi_ifc SWI Interface

The SWI Interface module provides interfaces for implementing software interrupts (SWI) within the Sidewalk SDK. These interfaces enable developers to handle asynchronous events and improve the responsiveness of their applications by leveraging platform-specific SWI mechanisms.

**⚠ WARNING ⚠**: SWI thread priority in RTOS must stay high enough if you don't want to miss radio event

@defgroup sid_pal_critical_region_ifc Critical Region Interface

Provides API's used by components of the Sidewalk SDK to solve concurrency issues.

@defgroup sid_pal_radio_ifc Radio Interfaces

## Radio Interfaces

The Radio Interfaces module provides interfaces for interacting with radio hardware within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to radio functionalities, such as transmission, reception, and configuration of radio parameters. Developers can create applications that are portable across different hardware platforms while maintaining consistent radio behavior and performance.

The Radio Interfaces module includes support for various radio operations, including:

- **Transmission and Reception**: Interfaces for sending and receiving data over the radio.
- **Configuration**: Interfaces for configuring radio parameters such as data rate, channel, and power settings.
- **Event Handling**: Mechanisms for handling radio events such as transmission completion, reception completion, and errors.
- **State Management**: Interfaces for managing the state of the radio, including transitions between different states such as sleep, standby, and active modes.

By utilizing the Radio Interfaces module, developers can ensure that their applications can effectively communicate over the radio within the Amazon Sidewalk network, regardless of the underlying hardware platform.

@defgroup sid_pal_radio_subghz_ifc Sub-GHz Interface

The Sub-GHz Interface module provides interfaces for interacting with Sub-GHz radio hardware within the Sidewalk SDK.

@defgroup sid_pal_radio_fsk_ifc FSK Interface

FSK modulation defines for Sidewalk.

@defgroup sid_pal_radio_lora_ifc LoRa Interface

LoRa modulation defines for Sidewalk.

@defgroup sid_pal_peripheral_ifc Peripheral Interfaces

## Peripheral Interfaces

The Peripheral Interfaces module provides interfaces for interacting with various hardware peripherals. These interfaces ensure consistent and platform-independent access to peripheral functionalities such as GPIO, temperature sensors, and timers. Developers can create applications that are portable across different hardware platforms while maintaining consistent behavior and performance.

@defgroup sid_pal_temp_ifc Temperature

The Temperature Interface module provides interfaces for interacting with temperature sensors within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to temperature measurement functionalities, allowing developers to create applications that can accurately monitor and respond to temperature changes across different hardware platforms.

@defgroup sid_pal_gpio_ifc GPIO

The GPIO Interface module provides interfaces for interacting with General-Purpose Input/Output (GPIO) pins within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to GPIO functionalities, allowing developers to control and monitor digital signals across different hardware platforms.

@defgroup sid_pal_sbus_ifc Serial Bus Interface

The Serial Bus Interface module provides interfaces for interacting with serial bus communication protocols within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to serial bus functionalities, allowing developers to implement communication between different components and peripherals across various hardware platforms.

@defgroup sid_pal_sclient_ifc Serial Client Interface

The Serial Client Interface module provides interfaces for implementing serial communication clients within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to serial communication functionalities, allowing developers to create applications that can communicate with various serial devices and peripherals across different hardware platforms.

@defgroup sid_pal_timers_ifc Timer Interfaces

## Timer Interfaces

The Timer Interfaces module provides interfaces for managing timer functionalities within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to timer operations, such as setting, starting, stopping, and resetting timers. Developers can create applications that are portable across different hardware platforms while maintaining consistent timing behavior and performance.

@defgroup sid_pal_delay_ifc Delay

Provides a way for Sub-Ghz protocol to control delay.

@defgroup sid_pal_uptime_ifc Uptime

The Uptime Interface module provides interfaces for tracking and managing system uptime within the Sidewalk SDK. These interfaces ensure consistent and platform-independent access to uptime information, allowing developers to monitor the duration for which the system has been running since the last reset or power cycle.

@defgroup sid_pal_timer_ifc Timer

Interface for timers for sidewalk SDK.

@defgroup sid_pal_log_ifc Logging Interface

## Logging Interface

Interface for log for sidewalk SDK.
For more details, please see [Logging Documentation](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-stack-structure/#logs) 

@defgroup sid_pal_storage_ifc Storage Interface

## Storage Interface

The Storage Interface module provides interfaces for managing persistent storage within the Sidewalk SDK. This module includes interfaces for key-value storage and manufacturing page storage. Developers can ensure consistent and platform-independent access to storage functionalities, enabling their applications to store and retrieve data reliably across different hardware platforms.

@defgroup sid_pal_mfg_ifc Manufacturing Page

Provides manufacturing store interface and implementation to store the manufacturing page into the EFR32 flash.

@defgroup sid_pal_kv_ifc KV Storage

Provides persistent storage interface for key mapped values.

@defgroup sid_pal Sidewalk PAL

## PAL

The Sidewalk Platform Abstraction Layer (PAL) provides a set of standardized interfaces that abstract the underlying hardware and platform-specific details. This layer ensures that the Sidewalk SDK can operate seamlessly across different hardware platforms by providing consistent APIs for common functionalities such as GPIO, timers, logging, and storage. Developers can create portable applications that maintain consistent behavior and performance across various hardware configurations.

@defgroup sidewalk_sdk_ble_adapter BLE Adaptation

## BLE Adaptation

The BLE adapter module provides the necessary interfaces and functionalities to integrate Bluetooth Low Energy (BLE) capabilities into applications using the Sidewalk SDK. This module includes support for BLE advertising, connection management, and data transmission. It also defines the configuration structures and callback mechanisms required to handle BLE events and operations. By utilizing the BLE adapter, developers can enable their devices to communicate over BLE within the Amazon Sidewalk network.

**Key Features:**

- **BLE Advertising**: Supports BLE advertising to broadcast data to nearby devices.
- **Connection Management**: Manages BLE connections, including establishing and terminating connections.
- **Data Transmission**: Facilitates data transmission over BLE, including sending and receiving data.
- **Event Handling**: Provides callback mechanisms to handle various BLE events such as connection, disconnection, and data reception.
- **Configuration Structures**: Defines configuration structures for setting up BLE parameters.
- **Security**: Supports secure BLE communication through encryption and authentication mechanisms.

@defgroup sidewalk_sdk_ble_adapter_support BLE adapter

@defgroup sid_pal_tmr_types Timer Types

@defgroup sid_pal_gpio GPIO

@defgroup sid_pal_nvm3_mngr NVM3 Manager