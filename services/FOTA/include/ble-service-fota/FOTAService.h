/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2020-2021 Embedded Planet
 * Copyright (c) 2020-2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

#ifndef FOTA_SERVICE_H
#define FOTA_SERVICE_H

#include "ble/BLE.h"

#if BLE_FEATURE_GATT_SERVER

#include "ble/common/UUID.h"
#include "ble/GattServer.h"
#include "ble/Gap.h"

#include "ChainableGapEventHandler.h"
#include "ChainableGattServerEventHandler.h"

#include "platform/Callback.h"
#include "platform/Span.h"
#include "platform/PlatformMutex.h"

#include "events/EventQueue.h"

#include "descriptors/CharacteristicUserDescriptionDescriptor.h"

/**
 * Maximum length of data (in bytes) that the FOTA service
 * can receive at one time.
 *
 * Typically MTU - 3 bytes for overhead
 */
#ifndef MBED_CONF_CORDIO_DESIRED_ATT_MTU
#define BLE_FOTA_SERVICE_MAX_DATA_LEN MBED_CONF_BLE_FOTA_SERVICE_BUFFER_SIZE
#else
#define BLE_FOTA_SERVICE_MAX_DATA_LEN (MBED_CONF_CORDIO_DESIRED_ATT_MTU - 3)
#endif

/**
 * UUIDs
 */
namespace uuids {
namespace FOTAService {

    extern const char BaseUUID[];
    extern const char OffsetUUID[];
    extern const char BinaryStreamUUID[];
    extern const char ControlUUID[];
    extern const char StatusUUID[];
    extern const char VersionUUID[];

}}

class FOTAService : private ble::GattServer::EventHandler,
                    private ble::Gap::EventHandler,
                    private mbed::NonCopyable<FOTAService> {

public:


    /**
     * As per Bluetooth Core specification V5.2, Vol 3, Part F, Table 3.4 (Error Codes)
     * ATT Error Codes between 0x80 and 0x9F are reserved for use by the application
     *
     * These error codes are valid for the FOTAService application layer in addition to those
     * defined in the GattAuthCallbackReply_t enum.
     */
    enum ApplicationError_t {
        AUTH_CALLBACK_REPLY_ATTERR_APP_BUSY             = 0x0190, /** Application is busy */
        AUTH_CALLBACK_REPLY_ATTERR_UNSUPPORTED_OPCODE   = 0x0191, /** Received unsupported control op code*/
        AUTH_CALLBACK_REPLY_ATTERR_HW_INHIBIT           = 0x0192, /** Hardware inhibited processing the op code */
        AUTH_CALLBACK_REPLY_ATTERR_LOW_BATTERY          = 0x0193, /** Low battery inhibited processing the op code */
        AUTH_CALLBACK_REPLY_ATTERR_OUT_OF_SYNC          = 0x0194, /** Transfer is out of sync, cannot process op code in this state */
        /* 0x0195 through 0x019F are reserved for future use by base FOTA service */
    };

    /**
     * FOTA standard op codes
     */
    enum OpCode_t {
        FOTA_NO_OP      = 0x00, /** No operation */
        FOTA_START      = 0x01, /** Initiate a FOTA update session */
        FOTA_STOP       = 0x02, /** Abort a FOTA update session */
        FOTA_COMMIT     = 0x03, /** End a FOTA update session and commit the update */
        /* Op Codes 0x04 through 0x40 are reserved for future use by the base FOTA service */
    };

    /**
     * FOTA-specific status codes
     */
    enum StatusCode_t{
        FOTA_STATUS_OK                      = 0x00, /** Neutral state */
        FOTA_STATUS_UPDATE_SUCCESSFUL       = 0x01, /** Used to communicate successful update */
        FOTA_STATUS_XOFF                    = 0x02, /** Flow control - pause flow */
        FOTA_STATUS_XON                     = 0x03, /** Flow control - resume flow */
        FOTA_STATUS_SYNC_LOST               = 0x04, /** Unexpected fragment ID received */
        FOTA_STATUS_UNSPECIFIED_ERROR       = 0x05, /** Unspecified error occurred */
        FOTA_STATUS_VALIDATION_FAILURE      = 0x06, /** Validation/verification of the update candidate failed */
        FOTA_STATUS_INSTALLATION_FAILURE    = 0x07, /** Failed to install firmware update candidate */
        FOTA_STATUS_OUT_OF_MEMORY           = 0x08, /** Underlying update candidate memory is full */
        FOTA_STATUS_MEMORY_ERROR            = 0x09, /** Error occurred in underlying memory device */
        FOTA_STATUS_HARDWARE_ERROR          = 0x0A, /** Hardware failure */
        FOTA_STATUS_NO_FOTA_SESSION         = 0x0B, /** No FOTA session started */
        /* Status codes 0x0B through 0x40 are reserved for future use by the base FOTA service */
    };

    /** Standard packet types */

    /**
     * Binary Stream Packet
     */
    class BinaryStreamPacket {

    public:

        /**
         * Construct a binary stream packet from a given buffer
         * @param[in] buffer Span of bytes to construct packet from
         */
        BinaryStreamPacket(mbed::Span<uint8_t> buffer) : _buffer(buffer) {
        }

        /**
         * Get the fragment ID of this packet
         */
        uint8_t get_fragment_id() {
            return _buffer[0];
        }

        /**
         * Get a Span to the firmware binary data
         */
        mbed::Span<uint8_t> get_data() {
            return _buffer.subspan(1);
        }

    private:
        /* Buffer */
        mbed::Span<uint8_t> _buffer;
    };

    struct EventHandler {

        virtual ~EventHandler() { }

        virtual StatusCode_t on_binary_stream_written(FOTAService &svc, mbed::Span<const uint8_t> buffer) {
            return FOTA_STATUS_OK;
        }

        virtual GattAuthCallbackReply_t on_control_written(FOTAService &svc, mbed::Span<const uint8_t> buffer) {
            return AUTH_CALLBACK_REPLY_SUCCESS;
        }

    };

public:

    /**
     * Instantiate a FOTAService instance
     * @param[in] ble BLE instance to host the FOTA service
     * @param[in] queue EventQueue to process events on
     * @param[in] chainable_gap_event_handler ChainableGapEventHandler object to register multiple Gap events
     * @param[in] chainable_gatt_server_event_handler ChainableGattServerEventHandler object to register multiple GattServer events
     * @param[in] protocol_version String describing FOTA protocol version
     * @param[in] fw_rev Optional, Current firmware revision string
     * @param[in] dev_desc Optional, Description of the device that this firmware is executed on
     *
     * @note The optional parameters MUST be supplied if your GattServer has multiple FOTAService
     * instances available. They are optional if your GattServer has only one FOTAService instance.
     * Each FOTAService must implement a firmware revision characteristic with an
     * associated characteristic user description descriptor that uniquely identifies
     * the device that executes the firmware targeted by the FOTAService.
     */
    FOTAService(BLE &ble, events::EventQueue &event_queue,
            ChainableGapEventHandler &chainable_gap_eh,
            ChainableGattServerEventHandler &chainable_gatt_server_eh,
            const char *protocol_version, const char *fw_rev = nullptr,
            const char *dev_desc = nullptr);

    virtual ~FOTAService();

    /**
     * Initializer
     *
     * Register event handlers and add service to given BLE instance.
     *
     * @return BLE_ERROR_NONE if initialization was successful
     */
    ble_error_t init();

    /**
     * Set event handler
     *
     * @param handler EventHandler object to handle events raised by the FOTA service
     */
    void set_event_handler(EventHandler* handler);

    /**
     * Get the service's event queue
     * @note this may be used by the FOTAService::EventHandler to queue events for
     * deferred processing.
     */
    const events::EventQueue&& get_event_queue() const {
        return _event_queue;
    }

    /**
     * Pause flow control
     * @param[in] fragment_id
     */
    void set_xoff();

    /**
     * Resume flow control
     */
    void set_xon();

    /**
     * Notify sync lost
     */
    void notify_sync_lost();

    /**
     * Notify status
     * @param[in] buf Span to status buffer to notify the FOTA client with
     */
    void notify_status(mbed::Span<const uint8_t> buf);

    /**
     * Start/enter a FOTA session
     */
    void start_fota_session(void);

    /**
     * Stop/exit a FOTA session
     */
    void stop_fota_session(void);

protected:

    void reset(void);

    void on_control_write_request(GattWriteAuthCallbackParams *write_request);

    /**
     * Internal handler for BSC writes
     */
    void on_bsc_written(mbed::Span<const uint8_t> data);

    /** GAP EventHandler overrides */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

    /** Gatt Server EventHandler overrides */
    void onDataWritten(const GattWriteCallbackParams &params) override;

protected:

    BLE &_ble;
    events::EventQueue &_event_queue;
    ChainableGapEventHandler &_chainable_gap_eh;
    ChainableGattServerEventHandler &_chainable_gatt_server_eh;

    /** RX Buffer for binary serial */
    uint8_t _rxbuf[BLE_FOTA_SERVICE_MAX_DATA_LEN] = { 0 };

    /** FOTA control */
    uint8_t _control[MBED_CONF_BLE_SERVICE_FOTA_CONTROL_BUFFER_SIZE] = { 0 };

    /** Update status */
    uint8_t _status[MBED_CONF_BLE_SERVICE_FOTA_STATUS_BUFFER_SIZE] = { 0 };

    /** FOTA protocol version string */
    const char *_protocol_version_str;

    /** Optional firmware revision and description strings */
    const char *_fw_rev_str;

    /** Optional firmware characteristic user description descriptor */
    CharacteristicUserDescriptionDescriptor _fw_cudd;

    /** GattCharacteristic constructor requires a list of pointers to descriptors... */
    GattAttribute *_fw_descs[1] = { (GattAttribute *) &_fw_cudd };

    /** Gatt Characteristics */
    GattCharacteristic _binary_stream_char;
    GattCharacteristic _ctrl_char;
    GattCharacteristic _status_char;
    GattCharacteristic _protocol_version_char;
    GattCharacteristic _firmware_rev_char;

    EventHandler *_eh = nullptr;

    bool _fota_in_session = false;
    uint8_t _fragment_id = 0;
    bool _flow_paused = false;
    bool _sync_lost = false;
};

#endif /* BLE_FEATURE_GATT_SERVER */


#endif /* FOTA_SERVICE_H */
