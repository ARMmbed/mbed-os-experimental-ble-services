# FOTA Service Overview

The FOTA (**F**irmware **O**ver **T**he **A**ir) service facilitates transfer of firmware updates over BLE using GATT Characteristics.

The FOTA service has been designed to allow for extensions to optimize transfer speeds and enable advanced update features. These advanced features are not included in the base implementation in order to maximize compatibility with constrained targets.

## Terminology

FOTA stands for firmware over the air
The **FOTA Target** is the device hosting the FOTA Service
The **FOTA Client** is the device acting as the client of the FOTA Service

# FOTA Service Structure

The structure of the FOTA Service is below:

![fota-service.png](img/fota-service.png)

There are three mandatory characteristics and one optional characteristic.

## Binary Stream Characteristic

The Binary Stream Characteristic (**BSC**) is the primary characteristic used to transfer firmware binary information over BLE. The maximum length of this characteristic is limited by the MTU of the connection. To maximize transfer speeds, this characteristic can only be written to using the **write without response** operation. Since firmware transfers **must** be lossless and retain the original data sequence, the FOTA service implements an application-level error correction scheme in the event of a packet loss. Corrupt data detection (ie: CRC checks) are implemented in lower layers of the BLE protocol and therefore are not required in the application layer.

The packet structure of the the FOTA service is simple. The first byte of each write to the BSC must be a sequential, 8-bit **fragment ID**. The fragment ID must be incremented for each write to the BSC. The fragment ID must roll over from 255 to 0.

Subsequent bytes written to the BSC after the fragment ID are considered binary firmware data.

Writes to the BSC may be ignored if the FOTA target is busy (eg: waiting for flash operations to finish) and unable to buffer any more data. This busy status will be communicated to the FOTA client using the FOTA Status Characteristic described later.

Out-of-sequence writes to the BSC will be ignored and trigger a lost-synchronization response through the FOTA Status Characteristic. This implements the previously-mentioned error correction scheme. This mechanism allows for lost packets to be retransmitted and to reestablish synchronization between the FOTA target and client.

## FOTA Control Characteristic

The FOTA Control Characteristic allows the FOTA client to control the FOTA session. This characteristic has a variable length with a maximum length limited by the MTU of the connection. This allows the application to add more advanced commands by simply defining a new command operation code (op-code) and packet structure. 

This characteristic only supports the **write** (with response) operation. In accordance with Bluetooth Core specification V5.2, Vol 3, Part F, Table 3.4 (Error Codes), ATT Error Codes between 0x80 and 0x9F are reserved for use by the application. Some of these error codes are defined by the standard FOTA service implementation. Unreserved ATT Error Codes may be defined by user application extensions as necessary.

The application ATT error codes defined/reserved by the FOTA Service are tabulated below:

| ATT Error Code   | Name                                            | Description                                      |
|------------------|-------------------------------------------------|--------------------------------------------------|
| 0x0190           | `AUTH_CALLBACK_REPLY_ATTERR_APP_BUSY`           | FOTA Service is busy                             |
| 0x0191           | `AUTH_CALLBACK_REPLY_ATTERR_UNSUPPORTED_OPCODE` | Op Code is unsupported                           |
| 0x0192 to 0x019F | Reserved                                        | Reserved for future use by the base FOTA service |

Any unhandled op-codes will trigger a write response error code of `AUTH_CALLBACK_REPLY_ATTERR_UNSUPPORTED_OPCODE`.

Most standard operations use only 1 byte and have no parameters. The first byte written to the FOTA Control Characteristic is the command op-code. 

Several standard op-codes are defined and support for them is mandatory. The application is free to use any unreserved op-codes to extend the available FOTA features.

The standard op-codes are:

| Op Code   | Name        | Description                                     |
|-----------|-------------|-------------------------------------------------|
| 0x00      | No op       | No operation                                    |
| 0x01      | FOTA Start  | Initiate a FOTA update session                  |
| 0x02      | FOTA Stop   | Abort a FOTA update session                     |
| 0x03      | FOTA Commit | End a FOTA update session and commit the update |
| 0x04-0x40 | Reserved    | Reserved for future use by base FOTA service    |

An example of an extension op-code that could be added by the application is the ability to select from multiple "memory slots" where the binary stream can be written to.

### FOTA Start

The FOTA Start command starts a FOTA update session. This allows the user's application to perform any necessary preparations before beginning an update session. eg: shutdown other subsystems, close files, update the UI to indicate the update to the device user, erase flash memory in preparation to receive the update binary, etc.

**Tip:** If you plan to erase a large area of flash when preparing for a FOTA session, you should consider issuing many small erase operations to the flash rather than one large operation. This allows a single-thread (ie: baremetal) application to process any BLE events in between flash erase operations. This prevents the BLE connection from timing out because a large flash erase was blocking BLE processing for too long.

The FOTA target may choose to issue an immediate `XOFF` status notification to pause binary transmission until all preparations have been completed.

In this case, the FOTA client should not commence binary transmission until the FOTA target has issued an `XON` status notification. See the FOTA Status Characteristic section below for more information on software flow control.

### FOTA Stop

### FOTA Commit

## FOTA Status Characteristic

The FOTA Status Characteristic communicates status and op-code response information from the FOTA target to the FOTA client. This characteristic supports read and notify operations. The FOTA client should enable notifications from this characteristic to ensure status updates and error conditions are communicated as quickly as possible.

Similar to the FOTA Control Characteristic, the FOTA Status Characteristic has a variable length limited by the MTU of the connection. Again, this is to allow for user applications to define extensions to the FOTA service.

The base FOTA service implementation defines standard status codes that all FOTA clients should recognize. The application is free to use any unreserved status codes to extend the avaialable FOTA features.

The standard status codes are:

| Status Code | Name                 | Description                                      |
|-------------|----------------------|--------------------------------------------------|
| 0x00        | OK                   | Indicates a neutral ready state                  |
| 0x01        | UPDATE SUCCESS       | Update process was successful                    |
| 0x02        | XOFF                 | Software flow control, pause writing to the BSC  |
| 0x03        | XON                  | Software flow control, resume writing to the BSC |
| 0x04        | SYNC LOST            | Out of sync fragment ID received                 |
| 0x05        | UNSPECIFIED ERROR    | Unspecified error                                |
| 0x06        | VALIDATION FAILURE   | Failed to validate firmware update candidate     |
| 0x07        | INSTALLATION FAILURE | Failed to install firmware update candidate      |
| 0x08        | OUT OF MEMORY        | The underlying memory is full                    |
| 0x09        | MEMORY ERROR         | Error writing to underlying memory device        |
| 0x0A        | HARDWARE ERROR       | Hardware failure                                 |
| 0x0B-0x40   | Reserved             | Reserved for future use by base FOTA service     |

### Software Flow Control

If the FOTA target is unable to accept any more binary data during the FOTA session, the FOTA target will write the `XOFF` status code to the FOTA Status Characteristic. Any writes to the BSC after the `XOFF` status code has been sent will be ignored and the FOTA target will subsequently write `XOFF` to the FOTA Status Characteristic again. This is to handle the case where the FOTA client missed a previous `XOFF` status notification.

When the FOTA target is again able to accept new binary data, the FOTA target will write the `XON` status code to the FOTA Status Characteristic. It is recommended that the FOTA client periodically read the FOTA Status Characteristic when binary transmissio has been paused to ensure it will receive the `XON` status update. This is to handle the case where the FOTA client missed a previous `XON` status notification.

### Resychronization

Each write to the BSC must begin with the sequential 8-bit fragment ID of the transfer. Since write-without-response is used, it is possible for a BSC packet to be dropped. To detect this, the FOTA target caches the last fragment ID it received successfully. If the next fragment ID received is not this cached value + 1 (or 0 in the case of a fragment ID rollover from 255), the FOTA target will write the `SYNC_LOST` status code to the FOTA Status Characteristic. The FOTA target will also write the expected fragment ID as the second byte in the FOTA Status Characteristic.

The FOTA client must recognize this condition and correct it by returning to the appropriate location in the firmware binary being transferred  (as indicated by the expected fragment ID sent by the FOTA target) and resume transmission from there.

If the FOTA client misses a `SYNC_LOST` notification, the FOTA target will issue another `SYNC_LOST` notification upon reception of the next (out of sync) packet written to the BSC. A possible edge case in this scenario is if the FOTA client finishes writing the firmware binary without successfully receiving a `SYNC_LOST` notification. To catch this error, the FOTA client must read the FOTA Status Characteristic before attempting to issue a `FOTA Commit` command, and correct the out-of-sync condition as described earlier.

If the FOTA client attempts to issue a `FOTA Commit` command while the FOTA target is in an out-of-sync state, the FOTA target will issue another `SYNC_LOST` notification.

## Firmware Revision String Characteristic

The FOTA service specification includes an optional* Firmware Revision String Characteristic. This characteristic is a standard, BT-SIG specified BLE GATT Characteristic with a 16-bit UUID of `0x2A26`. This characteristic is **only optional** if:

- There is only **one** FOTA service instance in the device's GATT Profile **AND**
- The device's GATT profile has **one** instance of the standard Device Information Service (UUID: 0x180A) that includes **one** instance of the Firmware Revision String Characteristic (UUID: 0x2A26), and this characteristic is populated with the revision string of the firmware associated with this FOTA service.

The intent behind this is that the firmware revision associated with a FOTA service is always available. If there is any ambiguity (eg: the GATT profile has two Firmware Revision String Characteristics in its structure), the FOTA service Firmware Revision String Characteristic is **mandatory**.

Additionally, there is a conditionally optional Characteristic User Description Descriptor (CUDD, UUID: 0x2901). This descriptor is optional **if and only if** there is only one Firmware Revision String Characteristic available on the server (and therefore only one FOTA service instance as well).

If there are multiple FOTA services or multiple Firmware Revision String characteristics, this descriptor is **mandatory**.

The contents of this descriptor should describe the module that this FOTA service instance updates the firmware of, eg: "cellular modem", "coprocessor", etc.

## Documentation Tools

Tools used to generate this documentation include PlantUML, LaTeX, and [tablesgenerator.com](https://www.tablesgenerator.com/markdown_tables#).

You can edit the tables more easily by copying them into the above Markdown tables generator.
