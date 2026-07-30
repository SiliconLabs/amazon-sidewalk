# Amazon Sidewalk - SoC Qualification

The SoC Qualification example is a command-line interface (CLI) based sample application. It enables testing of the Sidewalk API functions via the command line. This is the reference application for Sidewalk qualification and contains all the commands needed to pass the certification with Amazon.

Amazon fully manages the qualification process. For any questions regarding this process, please reach out to Amazon support directly. This sample application serves as a guide for developers to implement the interface with Amazon's qualification tests. The qualification sample application can be used to port custom hardware and should be included in the application submitted to Amazon for approval.

> **Ⓘ INFO Ⓘ**: This application can be used with the KG100S module or Sidewalk-supported EFR32 series 2 SoCs with at least 768kB flash size.

> **⚠ WARNING ⚠**: Sub-GHz communication occurs in the 900MHz band, a frequency open in the US but may be restricted in other regions.

## Prerequisites

To successfully interface with Amazon Sidewalk, this example application requires the preparation of cloud (AWS) resources and the addition of device credentials matched to those resources. To perform these tasks and procure access to a Sidewalk gateway, complete the initial software and hardware setup steps described in [Getting Started: Prerequisites](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites).

> **Ⓘ INFO Ⓘ**: Make note of the additional sub-GHz considerations discussed in the [Silicon Labs Wireless Development Kit](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/prerequisites#silicon-labs-wireless-development-kit) section of the hardware prequisites.

## Build the Application

With prerequisites in place, generate the primary application image as described in [Getting Started: Create and Compile your Sidewalk Application](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/create-and-compile-application).

## Prepare the Cloud and Endpoint

Create AWS resources to interface with your endpoint and couple the application image with device-specific credentials by following the steps in [Getting Started: Provision your Amazon Sidewalk Device](https://docs.silabs.com/amazon-sidewalk/latest/sidewalk-getting-started/provision-your-device).

## Interacting with the Endpoint

On boot, Sidewalk is not running, the application is waiting for a user input. Several commands are available to initialize and start the Sidewalk stack and interact with its configuration parameters. For each CLI call, there is a corresponding API call in the Sidewalk stack. For more details on each CLI call, refer to the [Amazon Sidewalk Specifications](https://docs.sidewalk.amazon/specifications/).

The J-Link RTT interface provides access to the CLI commands. The following table shows the available commands and their effect. Some commands are dependent on the currently selected radio layer.

### Initialize the stack

```
    sid init <1,2,3,4,5,6,7,8>                      - Initialize the Sidewalk stack. This calls the sid_init() API.
                                                      1 is SID_LINK_TYPE_1 (BLE)
                                                      2 is SID_LINK_TYPE_2 (FSK)
                                                      3 is SID_LINK_TYPE_3 (CSS)
                                                      4 is SID_LINK_TYPE_1 | SID_LINK_TYPE_3 (BLE + CSS)
                                                      5 is SID_LINK_TYPE_1 | SID_LINK_TYPE_2 (BLE + FSK)
                                                      6 is SID_LINK_TYPE_2 | SID_LINK_TYPE_3 (FSK + CSS)
                                                      7 is SID_LINK_TYPE_1 | SID_LINK_TYPE_2 | SID_LINK_TYPE_3 (BLE + FSK + CSS)
                                                      8 is SID_LINK_TYPE_ANY

    sid set_init_cfg <metrics> <fsk_ffs> <ffs_retry_period>
                                                    - Sets the init configuration of Sidewalk stack. This command needs to be run before every sid init for the new setting to take effect
                                                       <metrics> enable/disable Sub-Ghz metrics reporting to the cloud services (1 enable, 0 disable)
                                                       <fsk_ffs> enable/disable of FFS of Sub-Ghz FSK link only. Does not apply for BLE (1 enable, 0 disable)
                                                       <ffs_retry_period> Retry periodicity of FFS over FSK in seconds

    sid deinit                                      - Deinitialize the sidewalk stack, calls sid_deinit() API
```

### Start the stack

```
    sid start  <link>                               - Dtart the sidewalk stack, calls sid_start() API.
                                                      The link value is optional, it can take the same values as for sid init command. 
                                                      If link value is not present the one set with sid init will be used to call sid_start api.

    sid stop   <link>                               - stop the sidewalk stack, calls sid_stop() API.
                                                      The link value is optional, it can take the same values as for sid init command. 
                                                      If link value is not present the one set with sid init will be used to call sid_stop api.
```

### Send a message

```
    sid send -t <tv> -d <dv> -l <lm> -i <id> -o <low> -a <ack> <retry> <ttl> -r <data>
                                                    - send data over the SID_LINK_TYPE selected, calls the sid_put_msg()API.
                                                      Data field must always be placed at the end of command patametrs.
                                                      Example usage:
                                                          - sid send TEST
                                                          - sid send -t 0 -d 2 TEST
                                                          - sid send -t 1 -d 1 -r 01AAFF02
                                                      Optional parametrs:
                                                    - t option force specific message type to be set in message descriptor
                                                        possible <tv> values:
                                                        0 - SID_MSG_TYPE_GET
                                                        1 - SID_MSG_TYPE_SET
                                                        2 - SID_MSG_TYPE_NOTIFY
                                                        3 - SID_MSG_TYPE_RESPONSE
                                                        If -t option is not used message type in message descriptor will be set to SID_MSG_TYPE_NOTIFY.
                                                        If message type is set to RESPONSE type ID for outgoing message will be set with value that was previously received with GET type message. Or it can be set with sid set_rsp_id command.
                                                    - d specifies the destination
                                                        possible <dv> values:
                                                        1 - SID_LINK_MODE_CLOUD
                                                        2 - SID_LINK_MODE_MOBILE
                                                        If -d option is not used link_mode in message descriptor will be set to SID_LINK_MODE_CLOUD.
                                                    - l link mask on which message should be sent, if not set LINK_TYPE_ANY will be used
                                                        <lm> link mask - for possible values see 'sid init' command
                                                    - i message id that needs to be used to send response. Valid only for messages of type response
                                                        <id> response id
                                                    - o low laency configuration
                                                        possible <low> values:
                                                        0 - Default Setting. Send the message with low latency disabled.
                                                        1 - Send the message as a CSS low latency message
                                                    - a configure parameters for transport ack:
                                                        <ack> - enable/disable ACK
                                                          1 - enable ACK
                                                          0 - disable ACK
                                                        <retry> - number of retry. 0 ~ 255
                                                        <ttl> - total seconds the stack holds the message in its queue. 0 ~ 65535
                                                    - r data is interpreted hex string e.g. 010203AAFF
                                                      If -r parameter is not present data field is treated as ascii.
```

### Change the configuration at runtime

```
    sid option <option> <val1>...<valN>             - Set link options. This calls the sid_option() API. 
                                                      Possible inputs for  "<option> <val1>...<valN>" are as follows:
                                                      "-b <batlevel>" - for setting battery level for SID_LINK_TYPE_1. Not supported for other link types.
                                                          <batlevel> - (uint8) battery level.
                                                      "-lp_set <val1> .. <valN>" -  sets SID_LINK_TYPE_2 and SID_LINK_TYPE_3 (900 MHz) link profile and related parameters.
                                                        This API requires the device having network time (GCS) and returns an error code otherwise.
                                                        This API can be exercised only when the link is started otherwise, an error code is returned
                                                        At bootup time the API returns an error code prior to obtaining GCS.
                                                        Example usage:
                                                            "-lp_set 1" - Set FSK power profile 1.
                                                            "-lp_set 2 <rx_int>" - Set FSK power profile 2 with optional rx_int parameter".
                                                          <rx_int> Specifies DL interval between rx opportunities in units of ms. The value must be a multiple of 63ms. When ommitted the default value of 63ms is used.
                                                            "-lp_set 0x80 <rxwc>" - Set CSS power profile A, where <rxwc> is the rx_window count parameter.
                                                            "-lp_set 0x81 <rxwc>" - Set CSS power profile B, where <rxwc> is the rx_window count parameter.
                                                          <rxwc> - (uint8) rx window count. 0 represents infinite windows.
                                                            "-lp_get_l2" - Gets link profile and associated parameters for FSK. 
                                                                Ex: "app: CMD: ERR: 0 Link_profile ID: 1 Wndw_cnt: 0" for power profile 1 and infinite RX windows
                                                            "-lp_get_l3" - Gets link profile and associated parameters for CSS. 
                                                                Ex: "app: CMD: ERR: 0 Link_profile ID: 128 Wndw_cnt: 5". for power profile A and 5 RX windows
                                                      "-d <0,1>" - filter duplicate message.
                                                          0 - filter duplicate message.
                                                          1 - don't filter duplicate message and notify to user.
                                                      "-gd" - Get filter duplicates configuration. 
                                                            Ex: "app: CMD: ERR: 0 Filter Duplicates: 0"
                                                      "-m <policy>" - Set link connection policy
                                                          <policy> value of the link connection policy
                                                          0 - SID_LINK_CONNECTION_POLICY_NONE
                                                          1 - SID_LINK_CONNECTION_POLICY_AUTO_CONNECT
                                                          2 - SID_LINK_CONNECTION_POLICY_MLM
                                                      "-gm" - Get current link connection policy
                                                      "-c <link> <enable> <priority> <timeout>" - Set Auto connect policy parameters per link
                                                          <link_type> - link on which the auto connect parameters need to be applied. Valid values are only (1,2,3)
                                                          1 - SID_LINK_TYPE_1 (BLE)
                                                          2 - SID_LINK_TYPE_2 (FSK)
                                                          3 - SID_LINK_TYPE_3 (CSS)
                                                          <enable> - enable/disable auto connect for the link type
                                                          0 - Disable auto connect
                                                          1 - Enable auto connect
                                                          <priority> - priority per link, valid values 0(Highest) to 255(Lowest), optional when disabling auto connect
                                                          <timeout> - total seconds the stack attempts to establish a connection on the link, optional when disabling auto connect
                                                       "-gc <link_type>" - Get Auto connect policy per link.
                                                       "-ml <policy>" - Set Multi link policy parameters
                                                            <policy> - The multi link policy that needs to be applied. valid values are only (0,1,2,3,4)
                                                            0 - SID_LINK_MULTI_LINK_POLICY_DEFAULT
                                                            1 - SID_LINK_MULTI_LINK_POLICY_POWER_SAVE
                                                            2 - SID_LINK_MULTI_LINK_POLICY_PERFORMANCE
                                                            3 - SID_LINK_MULTI_LINK_POLICY_LATENCY
                                                            4 - SID_LINK_MULTI_LINK_POLICY_RELIABILITY
                                                        "-gml" - Get current configured Multi link policy.
                                                      "-st_get" - Get statistics
                                                        Ex: "app: CMD: ERR: 0 tx: 3, acks_sent 8, tx_fail: 0, retries: 4, dups: 6, acks_recv: 7 rx: 8"
                                                      "-st_clear" - Clear statistics
                                                      "-gsi" - Get Sidewalk ID
                                                        Ex: "<info> app: CMD: ERR: 0 SIDEWALK_ID: BFFFFFC94E"
```

### BLE-only commands

```
    sid conn_req <0,1>                              - set the connection request bit in BLE beacon,
                                                      1 - set connection request bit
                                                      0 - clear connection request bit
                                                      This calls the sid_ble_bcn_connection_request() API.
```

### FSK-only commands

```
    gwscan start [<duration_sec>] [<max_gw_num>]    - Start GW scaning tool, this can only be done before sid init. This tool is only available for FSK-enabled builds.
                                                        Format: gwscan start [<duration_sec>] [<max_gw_num>]
                                                        Parameters:
                                                            <duration_sec> -[optional] This parameter is used to specify the scanning duration. Range: 1-65535, default value: 0 (infinity)
                                                            <max_gw_num>  -[optional] Maximum number of discovered gateways, range: 1-1000, default value: 100
                                                            If the parameters are not specified, then scanning will continue until it is stopped by stop command.

    gwscan stop                                     - Stop GW scaning tool and print results
                                                        Format: gwscan stop
```

### Testing commands

```
    sid pwr_meas_start <un> <ul> <sd> <mt> <mode>   - trigger power measurement mode, command takes control over main loop, allows to go to idle state and put
                                                      device into sleep mode, during command execution CLI is not processed and not available, every 1 second
                                                      command can add uplink packets to send queue
                                                      un    - number of uplink packets generated during command execution, max 255
                                                      ul    - length of one uplink packet, max 255, it accompanies with number of uplink packets
                                                      sd    - start delay time in seconds, after this time command will start to add ul packets to send queue  max 4000 sec
                                                      mt    - measurement time in seconds, time after which command will end, max 4000 sec, it accompanies with start delay time
                                                      mode  - indicates the behavior of the command
                                                                0 - sends packets defined by <un> and <ul> only. The interface shall be started and connection established e.g. for BLE by "sid start 1" and "sid conn_req 1" commands before the command is called. The value is applicable for all link types: BLE, FSK and CSS.
                                                                1 - starts the interface and initiates the connection (like commands sid start 1, sid conn_req 1) before packets defined by <un> and <ul> are sent. It also stops the interface when the measurement is ended (like sid stop). The value is applicable for BLE link type only.
```

### Utility

```
    sid factory_reset                               - Factory reset the board, deleting all registration status. This calls the
                                                      sid_set_factory_reset() API.

    sid get_mtu <1,2,3>                             - Get the MTU for the selected link type. This calls the sid_get_mtu() API.
                                                      1 is SID_LINK_TYPE_1 (BLE)
                                                      2 is SID_LINK_TYPE_2 (FSK)
                                                      3 is SID_LINK_TYPE_3 (CSS)
                                                      8 is SID_LINK_TYPE_ANY.

    sid last_status                                 - Get last status of Sidewalk library. Result is printed in the same format as
                                                      EVENT SID STATUS. This call the sid_get_status() API.

    sid get_time <0>                                - get time from Sidewalk library. This calls the sid_get_time() API.
                                                      0 - GPS time

    sid set_dst_id <id>                             - set message destination ID. This calls the sid_set_msg_dest_id() API.

    sid sdk_config                                  - Prints the various sid sdk config values
```

### Print Commands

```
print - Print values from device
    fwversion     - Print FW version
    mfg           - Print mfg values
    txid          - Print Obfuscated device ID (TX-ID)
```

## Migration to Sidewalk 2.9.0

Starting with Sidewalk 2.9.0, the manufacturing store configuration in `app_main.c` must provide a **function pointer** for `app_value_to_offset` in `sid_pal_mfg_store_region_t`. Assigning the literal `0xffffffff` to that field is no longer valid.

This change is **required for CMake and LLVM (Clang) builds**. Those toolchains enforce correct function-pointer types in struct initializers; treating `app_value_to_offset` as an integer constant will fail to compile or produce errors under stricter type checking.

If your project does not extend the manufacturing store with application-specific values, add a stub implementation that returns `SID_PAL_MFG_STORE_INVALID_OFFSET` (`0xFFFFFFFF`) for every value, and wire it into `mfg_config` as shown below.

**Before (Sidewalk 2.8.2 and earlier):**

```c
static const sid_pal_mfg_store_region_t mfg_config = {
  .app_value_to_offset = 0xffffffff,
};
```

**After (Sidewalk 2.9.0):**

```c
uint32_t mfg_store_app_value_to_offset(int value)
{
  (void)value;
  return 0xFFFFFFFF;  /* SID_PAL_MFG_STORE_INVALID_OFFSET — no app-specific mfg values */
}

static const sid_pal_mfg_store_region_t mfg_config = {
  .app_value_to_offset = mfg_store_app_value_to_offset,
};
```

If your application **does** map custom manufacturing values (identifiers greater than `SID_PAL_MFG_STORE_CORE_VALUE_MAX`), replace the stub body with your existing offset logic instead of always returning `0xFFFFFFFF`. The function signature must match `sid_pal_mfg_store_app_value_to_offset_t` in `sid_pal_mfg_store_ifc.h`.

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com).
