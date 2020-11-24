/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2020 Embedded Planet
 * Copyright (c) 2020 ARM Limited
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

#ifndef MBED_OS_EXPERIMENTAL_BLE_SERVICES_SERVICES_INC_DFUSERVICE_H_
#define MBED_OS_EXPERIMENTAL_BLE_SERVICES_SERVICES_INC_DFUSERVICE_H_

#include "ble/common/UUID.h"
#include "ble/BLE.h"
#include "ble/GattServer.h"
#include "ble/Gap.h"

#include "platform/Callback.h"
#include "platform/Span.h"
#include "platform/CircularBuffer.h"
#include "platform/PlatformMutex.h"

#include "events/EventQueue.h"

#include "BlockDevice.h"

#include "descriptors/CharacteristicUserDescriptionDescriptor.h"

/**
 * Maximum length of data (in bytes) that the DFU service
 * can receive at one time.
 *
 * Typically MTU - 3 bytes for overhead
 */
#ifndef MBED_CONF_CORDIO_DESIRED_ATT_MTU
#define BLE_DFU_SERVICE_MAX_DATA_LEN MBED_CONF_BLE_DFU_SERVICE_BUFFER_SIZE
#else
#define BLE_DFU_SERVICE_MAX_DATA_LEN (MBED_CONF_CORDIO_DESIRED_ATT_MTU - 3)
#endif

/**
 * RX Buffer Size
 * This should be at least 1.5X the maximum MTU you expect to accept
 * Defaults to 2.5X maximum MTU
 */
#ifndef MBED_CONF_BLE_DFU_SERVICE_RX_BUFFER_SIZE
#define MBED_CONF_BLE_DFU_SERVICE_RX_BUFFER_SIZE ((BLE_DFU_SERVICE_MAX_DATA_LEN << 1) +\
                                                  (BLE_DFU_SERVICE_MAX_DATA_LEN >> 1))
#endif

/**
 * RX Buffer Flow Control Pause Threshold
 * This is the maximum number of bytes that can be placed into the RX buffer
 * before the server automatically sets the flow control pause bit.
 *
 * By default this is 2X the maximum MTU
 */
#ifndef MBED_CONF_BLE_DFU_SERVICE_RX_FC_PAUSE_THRESHOLD
#define MBED_CONF_BLE_DFU_SERVICE_RX_FC_PAUSE_THRESHOLD (BLE_DFU_SERVICE_MAX_DATA_LEN << 1)
#endif

/**
 * RX Buffer Flow Control Unpause Threshold
 * This is the maximum number of bytes that can be in the RX buffer
 * before the server automatically clears the flow control pause bit.
 *
 * By default this is 1X the maximum MTU
 */
#ifndef MBED_CONF_BLE_DFU_SERVICE_RX_FC_UNPAUSE_THRESHOLD
#define MBED_CONF_BLE_DFU_SERVICE_RX_FC_UNPAUSE_THRESHOLD BLE_DFU_SERVICE_MAX_DATA_LEN
#endif

/**
 * Maximum number of slots available
 */
#ifndef MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS
#define MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS 3
#endif

/**
 * Bit flags
 */
#define DFU_CTRL_ENABLE_BIT         (1 << 0)
#define DFU_CTRL_COMMIT_BIT         (1 << 1)
#define DFU_CTRL_DELTA_MODE_EN_BIT  (1 << 2)
#define DFU_CTRL_FC_PAUSE_BIT       (1 << 7)

/* Bitmask of read-only bits in the DFU Ctrl bit set */
#define DFU_CTRL_READONLY_BITS      (DFU_CTRL_FC_PAUSE_BIT)

/**
 * UUIDs
 */
namespace uuids {
namespace DFUService {

    extern const char BaseUUID[];
    extern const char SlotUUID[];
    extern const char OffsetUUID[];
    extern const char BinaryStreamUUID[];
    extern const char ControlUUID[];
    extern const char StatusUUID[];

}}


/**
 * API Brainstorm:
 * DFU service will have several characteristics:
 * - Current Offset (Read/Write), gives the offset address, in bytes, of the write pointer
 *      --- Writes to this characteristic while there is data in binary data stream buffer will be rejected.
 *      A rejected write will initiate flushing the buffer to the selected slot block device.
 *      Subsequent writes will be rejected until flushing is complete.
 *      Note: any writes to the binary data stream characteristic while the buffer is being flushed will be ignored
 *      --- If the delta bit is enabled, any memory sections skipped will be written with bytes copied from the primary application
 * - Binary Data stream, variable-length array characteristic for streaming the update in binary.
 *      The underlying block device will be written at the offset given by current offset for each byte written to this characteristic.
 *      The offset is incremented for each byte written
 * - DFU Control Characteristic
 *      - Notify/Indicate/Read (for flow control bit mainly)
 *      - Write (w/ response), ability to add security requirements
 *      - Bit flags:
 *      --- DFU Enable, DFU abort = write 0 during update
 *      --- DFU Commit
 *      --- Delta mode (any skipped sections will be written with existing app data)
 *      --- Flow Control Bit (if set, peer should pause writing to binary stream characteristic)
 *      - Write is only allowed if DFU is currently allowed
 *      - Allows application/device to prepare for an update (cache/save data, shutdown certain things, erase/prepare flash area)
 * - Status characteristic
 *      - Notify/Indicate/Read
 *      - Error code (eg: update too large, invalid update (after reboot), etc)
 *      - If highest bit is set it indicates a sync lost notification
 *      ---> The 7LSB will then indicate the expected sequence ID that did not match. The client should restart transmission from this sequence ID.
 * - Selected Slot
 *      - Write (w/ response)
 *      - Write is only allowed if slot has valid block device
 *      - Deselected slot BD is deinited, selected slot is inited
 *      - Similar to offset, writes to this characteristic while there is data in the binary data stream buffer
 *      will flush the buffer to the selected block device before the selected slot change is applied.
 *      Note: In delta mode, selecting a new slot WILL NOT result in the remaining data in the slot being written with copied application data
 *      To accomplish this, the peer should write the offset characteristic to the point where data should be copied before changing slots.
 *
 * Notes:
 * - Valid slots are intended to be empirically determined by the peer (as necessary)
 * by attempting to set the
 *
 * - Should writes to the binary data stream be synchronized with flash write waits? Potentially much slower
 * - Control bitflags class? Use std::bitset?
 *
 */
#if BLE_FEATURE_GATT_SERVER

class DFUService : public ble::GattServer::EventHandler,
                   public ble::Gap::EventHandler,
                   private mbed::NonCopyable<DFUService> {

public:


    /**
     * As per Bluetooth Core specification V5.2, Vol 3, Part F, Table 3.4 (Error Codes)
     * ATT Error Codes between 0x80 and 0x9F are reserved for use by the application
     *
     * These error codes are valid for the DFUService application layer in addition to those
     * defined in the GattAuthCallbackReply_t enum.
     */
    enum ApplicationError_t {
        AUTH_CALLBACK_REPLY_ATTERR_APP_NOT_ALLOWED      = 0x019C, /** Response when client attempts to enable DFU when disallowed */
        AUTH_CALLBACK_REPLY_ATTERR_APP_READONLY         = 0x019D, /** A write request was made that modifies data that is read-only */
        AUTH_CALLBACK_REPLY_ATTERR_APP_BUSY             = 0x019E, /** DFUService is busy (eg: flush in progress) */
        AUTH_CALLBACK_REPLY_ATTERR_APP_INVALID_SLOT_NUM = 0x019F, /** Client requested invalid slot index */
    };

    /**
     * DFU-specific status codes
     */
    enum StatusCode_t{
        DFU_STATE_IDLE                      = 0x00, /** Neutral state */
        DFU_STATE_UPDATE_SUCCESSFUL         = 0x01,
        DFU_STATE_UNKNOWN_FAILURE           = 0x02,
        DFU_STATE_VALIDATION_FAILURE        = 0x03, /** Validation/Authentication of update candidate failed */
        DFU_STATE_INSTALLATION_FAILURE      = 0x04, /** Installation of update candidate failed */
        DFU_STATE_APPLICATION_OVERSIZE      = 0x05, /** Update candidate exceeded memory bounds */
        DFU_STATE_FLASH_ERROR               = 0x06, /** Flash error */
        DFU_STATE_HARDWARE_ERROR            = 0x07, /** Hardware failure */

        DFU_STATE_SYNC_LOSS_BIT             = 0x80, /** If the MSbit is set in the status, the 7LSB indicate the sequence ID at which sync was lost */

    };

    /**
     * Class encapsulating a change to the DFU control characteristic
     */
    class ControlChange {

        /* Allow DFUService to instantiate ControlChange instances */
        friend DFUService;

    public:

        const DFUService& service() const {
            return _dfu_svc;
        }

        uint8_t value() const {
            return _new_value;
        }

        uint8_t get_changed_bits() const {
            return (_old_value ^ _new_value);
        }

    protected:

        ControlChange(DFUService& service, uint8_t value) :
            _dfu_svc(service), _old_value(service.get_dfu_control_bits()),
            _new_value(value) {
        }

    protected:

        DFUService& _dfu_svc;

        uint8_t _old_value;
        uint8_t _new_value;

    };

public:

    /**
     * Instantiate a DFUService instance
     * @param[in] bd BlockDevice to use for storing update candidates in slot 0
     * @param[in] queue EventQueue to process memory writes on
     * @param[in] fw_rev Optional, Current firmware revision string
     * @param[in] dev_desc Optional, Description of the device that this firmware is executed on
     *
     * @note The optional parameters MUST be supplied if your GattServer has multiple DFUService
     * instances available. They are optional if your GattServer has only one DFUService instance.
     * Each DFUService must implement a firmware revision characteristic with an
     * associated characteristic user description descriptor that uniquely identifies
     * the device that executes the firmware targeted by the DFUService.
     */
    DFUService(mbed::BlockDevice *bd, events::EventQueue &queue,
            const char *fw_rev = nullptr, const char *dev_desc = nullptr);

    virtual ~DFUService();

    uint8_t get_dfu_control_bits() const {
        return _dfu_control;
    }

    bool is_dfu_enabled() const {
        return (_dfu_control & DFU_CTRL_ENABLE_BIT);
    }

    void start(BLE &ble_interface);

    void assign_slot_block_device(uint8_t slot, mbed::BlockDevice *bd);

    /**
     * Register a callback to be executed when a write request occurs for the
     * DFU Control characteristic. The application may then accept or reject the
     * requested changes as appropriate.
     *
     * @param[in] cb Application callback or nullptr to deregister
     *
     * @note If the application does not explicitly reject the control request,
     * the request will be accepted by default.
     */
    void on_dfu_control_request(mbed::Callback<GattAuthCallbackReply_t(ControlChange&)> cb) {
        _ctrl_req_cb = cb;
    }

    /**
     * Register a callback to be executed when a write is committed to the
     * DFU Control characteristic
     *
     * @param[in] cb Application callback or nullptr to deregister
     *
     */
    void on_dfu_control_change(mbed::Callback<void(ControlChange&)> cb) {
        _ctrl_update_cb = cb;
    }

protected:

    /**
     * Initialize and erase the selected flash slot
     *
     * @note This function may run for several seconds while erasing the currently
     * selected slot, depending on the size of the slot and flash speed.
     */
    void init_selected_slot(void);

    /**
     * Internal function to process buffered binary serial data
     */
    void process_buffer(void);

    /**
     * Schedule a serialized call to process_buffer on the event queue.
     *
     * @note this function has no effect if a write has already been scheduled
     */
    void schedule_write(void) {
        if(!_scheduled_write) {
            _scheduled_write = _queue.call(
                    mbed::callback(this, &DFUService::process_buffer));
        }
    }

    /**
     * Set the status of the DFUService and notify any subscribed peers
     */
    void set_status(uint8_t status);

    /**
     * Set the DFU control characteristic and notify any subscribedd peers
     */
    void set_dfu_ctrl(uint8_t ctrl);

    /**
     * Sets the flow control pause bit and notifies any subscribed peers
     *
     * @note This will not have any effect if the bit is already set
     */
    inline void set_fc_bit(void) {
        mbed::ScopedLock<PlatformMutex> lock(_mutex);
        if(!(_dfu_control & DFU_CTRL_FC_PAUSE_BIT)) {
            set_dfu_ctrl(_dfu_control | DFU_CTRL_FC_PAUSE_BIT);
        }
    }

    /**
     * Clears the flow control pause bit and notifies any subscribed peers
     *
     * @note This will not have any effect if the bit is already cleared
     */
    inline void clear_fc_bit(void) {
        mbed::ScopedLock<PlatformMutex> lock(_mutex);
        if(_dfu_control & DFU_CTRL_FC_PAUSE_BIT) {
            set_dfu_ctrl(_dfu_control & ~(DFU_CTRL_FC_PAUSE_BIT));
        }
    }

    /**
     * Initiates a binary data stream buffer flush and sets the flow control bit
     */
    inline void initiate_flush(void) {
        mbed::ScopedLock<PlatformMutex> lock(_mutex);
        /* Initiate a flush and set the FC pause bit */
        _flush_bin_buf = true;
        set_fc_bit();
        schedule_write();
    }

    /**
     * Completes a binary data stream buffer flush and clears the flow control bit
     */
    void flush_complete(void) {
        mbed::ScopedLock<PlatformMutex> lock(_mutex);
        _flush_bin_buf = false;
        clear_fc_bit();
    }

    /** GattServer::EventHandler overrides */
    void onDataWritten(const GattWriteCallbackParams &params) override;

    void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) override;

    void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) override;

    /** Gap::EventHandler overrides */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

    /** Internal handlers */
    void on_slot_write_request(GattWriteAuthCallbackParams *params);
    void on_slot_written(uint8_t new_slot);

    void on_offset_write_request(GattWriteAuthCallbackParams *params);
    void on_offset_written(uint32_t new_offset);

    void on_bds_written(mbed::Span<const uint8_t> data);

    void on_dfu_ctrl_write_request(GattWriteAuthCallbackParams *params);
    void on_dfu_ctrl_written(uint8_t new_ctrl);

protected:

    /** Selected slot */
    uint8_t _selected_slot = 0;

    /** Current offset address */
    uint32_t _current_offset = 0;

    /** RX Buffer for binary serial */
    uint8_t _rxbuf[BLE_DFU_SERVICE_MAX_DATA_LEN] = { 0 };

    /** DFU control */
    uint8_t _dfu_control = 0;

    /** Update status */
    uint8_t _status = 0;

    /** Optional firmware revision and description strings */
    const char *_fw_rev_str;

    /** Optional firmware characteristic user description descriptor */
    CharacteristicUserDescriptionDescriptor _fw_cudd;

    /** GattCharacteristic constructor requires a list of pointers to descriptors... */
    GattAttribute *_fw_descs[1] = { (GattAttribute *) &_fw_cudd };

    /** Gatt Characteristics */
    GattCharacteristic _slot_char;
    GattCharacteristic _offset_char;
    GattCharacteristic _rx_char;
    GattCharacteristic _dfu_ctrl_char;
    GattCharacteristic _status_char;
    GattCharacteristic _firmware_rev_char;

    GattCharacteristic *_characteristics[6];

    GattService _dfu_service;

    GattServer *_server;

    /** Slot BlockDevices */
    mbed::BlockDevice *_slot_bds[MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS];

    /** Application callbacks */
    mbed::Callback<GattAuthCallbackReply_t(ControlChange&)> _ctrl_req_cb = nullptr;
    mbed::Callback<void(ControlChange&)> _ctrl_update_cb = nullptr;

    /** Internal circular buffer */
    mbed::CircularBuffer<uint8_t, MBED_CONF_BLE_DFU_SERVICE_RX_BUFFER_SIZE> _bin_stream_buf;

    /** Flush binary stream buffer flag */
    bool _flush_bin_buf = false;

    /** Mutex */
    PlatformMutex _mutex;

    events::EventQueue &_queue;

    /** Queued event ID for scheduling flash writes */
    uint32_t _scheduled_write = 0;

    /** Sequence ID for synchronization with client */
    uint8_t _seq_id = 0;

};

#endif //BLE_FEATURE_GATT_SERVER


#endif /* MBED_OS_EXPERIMENTAL_BLE_SERVICES_SERVICES_INC_DFUSERVICE_H_ */
