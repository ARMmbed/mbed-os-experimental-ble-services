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

/**
 * TODO check use of mutexes. May need to change to trying to lock in BLE handlers
 * to prevent the BLE thread/queue from being blocked for long enough to cause
 * a supervision timeout disconnection
 */

#ifndef MBED_CONF_BLE_DFU_SERVICE_TRACE_ENABLE
#define MBED_CONF_BLE_DFU_SERVICE_TRACE_ENABLE 0
#endif

#if MBED_CONF_BLE_DFU_SERVICE_TRACE_ENABLE || 1
#define TRACE_IF(x) do { x; } while(0);
#else
#define TRACE_IF(x)
#endif

#if BLE_FEATURE_GATT_SERVER

#include "DFUService.h"

#include "ble/gatt/GattCharacteristic.h"

#include "mbed_trace.h"

#include "platform/mbed_assert.h"
#include "platform/ScopedLock.h"

#define TRACE_GROUP "btdfu"

namespace uuids {
namespace DFUService {

     const char BaseUUID[]          = "53880000-65fd-4651-ba8e-91527f06c887";
     const char SlotUUID[]          = "53880001-65fd-4651-ba8e-91527f06c887";
     const char OffsetUUID[]        = "53880002-65fd-4651-ba8e-91527f06c887";
     const char BinaryStreamUUID[]  = "53880003-65fd-4651-ba8e-91527f06c887";
     const char ControlUUID[]       = "53880004-65fd-4651-ba8e-91527f06c887";
     const char StatusUUID[]        = "53880005-65fd-4651-ba8e-91527f06c887";

}}

DFUService::DFUService(mbed::BlockDevice *bd, events::EventQueue &queue, const char *fw_rev, const char *dev_desc) :
                _fw_rev_str(fw_rev), _fw_cudd(dev_desc),
                _slot_char(uuids::DFUService::SlotUUID, &_selected_slot, 1, 1,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
                         nullptr, 0, false),
                _offset_char(uuids::DFUService::OffsetUUID, (uint8_t*) &_current_offset, 4, 4,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
                         nullptr, 0, false),
                _rx_char(uuids::DFUService::BinaryStreamUUID, _rxbuf,
                        1, BLE_DFU_SERVICE_MAX_DATA_LEN,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE)),
                _dfu_ctrl_char(uuids::DFUService::ControlUUID, &_dfu_control, 1, 1,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)),
                _status_char(uuids::DFUService::StatusUUID, &_status, 1, 1,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)),
                /* FW Rev Char will be dropped by GattServer if val in nullptr, len is 0, and it's readable */
                _firmware_rev_char(GattCharacteristic::UUID_FIRMWARE_REVISION_STRING_CHAR,
                        (uint8_t *) fw_rev,
                        (fw_rev != nullptr) ? strlen(fw_rev) : 0, /* Min length */
                        (fw_rev != nullptr) ? strlen(fw_rev) : 0, /* Max length */
                        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                        _fw_descs,
                        (dev_desc != nullptr) ? 1 : 0,
                        true),
                _dfu_service(uuids::DFUService::BaseUUID,
                        _characteristics,
                        (fw_rev != nullptr) ?
                                (sizeof(_characteristics) / sizeof(_characteristics[0])) :      /** FW rev present */
                                (sizeof(_characteristics) / sizeof(_characteristics[0])) - 1),  /** FW rev absent */
                _server(nullptr),
                _queue(queue)
{

    _slot_bds[0] = bd;
    for(int i = 1; i < MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS; i++) {
        _slot_bds[i] = nullptr;
    }

    _characteristics[0] = &_slot_char;
    _characteristics[1] = &_offset_char;
    _characteristics[2] = &_rx_char;
    _characteristics[3] = &_dfu_ctrl_char;
    _characteristics[4] = &_status_char;

    /* Note: FW rev must always be at the end of the characteristics list since it is optional */
    _characteristics[5] = &_firmware_rev_char;

}

DFUService::~DFUService() {
}

void DFUService::start(BLE &ble_interface) {

    /** Setup write authorization callbacks */
    _slot_char.setWriteAuthorizationCallback(this, &DFUService::on_slot_write_request);
    _offset_char.setWriteAuthorizationCallback(this, &DFUService::on_offset_write_request);
    _dfu_ctrl_char.setWriteAuthorizationCallback(this, &DFUService::on_dfu_ctrl_write_request);

    /** Note: Characteristic authorization callbacks must be set BEFORE adding the service! */
    _server = &ble_interface.gattServer();
    _server->addService(_dfu_service);

}

void DFUService::assign_slot_block_device(uint8_t slot, mbed::BlockDevice *bd) {
    MBED_ASSERT(slot < MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS);
    _slot_bds[slot] = bd;
}

void DFUService::onDataWritten(const GattWriteCallbackParams &params) {
    if(params.handle == _slot_char.getValueHandle()) {
        on_slot_written(*params.data);
    } else
    if(params.handle == _offset_char.getValueHandle()) {
        uint32_t new_offset = 0;
        new_offset |= (params.data[0] << 24);
        new_offset |= (params.data[1] << 16);
        new_offset |= (params.data[2] << 8);
        new_offset |= params.data[3];
        on_offset_written(new_offset);
    } else
    if(params.handle == _rx_char.getValueHandle()) {
        on_bds_written(mbed::make_const_Span(params.data, params.len));
    } else
    if(params.handle == _dfu_ctrl_char.getValueHandle()) {
        on_dfu_ctrl_written(*params.data);
    }
}

void DFUService::onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) {
    if(params.attHandle == _dfu_ctrl_char.getValueHandle()) {
        TRACE_IF(tr_debug("Updates enabled for control characteristic"));
    } else if(params.attHandle == _status_char.getValueHandle()) {
        TRACE_IF(tr_debug("Updates enabled for status characteristic"))
    }
}

void DFUService::onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) {
    if(params.attHandle == _dfu_ctrl_char.getValueHandle()) {
        TRACE_IF(tr_debug("Updates disabled for control characteristic"));
    } else if(params.attHandle == _status_char.getValueHandle()) {
        TRACE_IF(tr_debug("Updates disabled for status characteristic"))
    }
}


void DFUService::set_status(uint8_t status) {
    TRACE_IF(tr_debug("notifying status: %d", status));
    _server->write(_status_char.getValueHandle(), &status, 1, false);
}

void DFUService::set_dfu_ctrl(uint8_t bits) {
    TRACE_IF(tr_debug("notifying ctrl: %d", bits));
    _server->write(_dfu_ctrl_char.getValueHandle(), &bits, 1, false);
}

void DFUService::onDisconnectionComplete(
        const ble::DisconnectionCompleteEvent &event) {
    // TODO determine disconnection behavior in various states
}

void DFUService::on_slot_write_request(GattWriteAuthCallbackParams *params) {
    /* Verify if desired slot is valid (within bounds and has valid BlockDevice */
    uint8_t desired_slot = *params->data;
    if(!(desired_slot < MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS) ||
       (_slot_bds[desired_slot] == nullptr)) {
        TRACE_IF(tr_debug("slot write request: rejected (invalid)"));
        params->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_APP_INVALID_SLOT_NUM;
    } else
    if(!_bin_stream_buf.empty() || _flush_bin_buf) {
        TRACE_IF(tr_debug("slot write request: rejected (busy)"));
        /* Reject slot write request and initiate a flush of the binary stream buffer */
        params->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_APP_BUSY;
        initiate_flush();
    } else {
        TRACE_IF(tr_debug("slot write request: accepted"));
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void DFUService::on_slot_written(uint8_t new_slot) {
    /* Ignore if selecting the same slot */
    if(_selected_slot != new_slot) {
        TRACE_IF(tr_debug("slot written: %d", new_slot));
        if(_slot_bds[new_slot] != nullptr) {
            mbed::ScopedLock<PlatformMutex> lock(_mutex);
            _slot_bds[_selected_slot]->deinit();
            _selected_slot = new_slot;
            /* Initialize and erase the selected slot */
            _queue.call(mbed::callback(this, &DFUService::init_selected_slot));
        }
    }
}

void DFUService::on_offset_write_request(GattWriteAuthCallbackParams *params) {
    if(!_bin_stream_buf.empty() || _flush_bin_buf) {
        TRACE_IF(tr_debug("offset write request: rejected (busy)"));
        /* Reject offset write request and initiate a flush of the binary stream buffer */
        params->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_APP_BUSY;
        initiate_flush();
    } else {
        TRACE_IF(tr_debug("offset write request: accepted"));
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void DFUService::on_offset_written(uint32_t new_offset) {
    TRACE_IF(tr_debug("offset written: %lu", new_offset));
    mbed::ScopedLock<PlatformMutex> lock(_mutex);
    _current_offset = new_offset;
}

void DFUService::on_bds_written(mbed::Span<const uint8_t> data) {

    uint8_t seq_id = *data.data();
    TRACE_IF(tr_debug("bds written, sequence num: %d, %i bytes in payload", seq_id, data.size()-1));

    /* Ignore 0-length writes */
    if((data.size() - 1) == 0) {
        TRACE_IF(tr_warn("zero-length packet written, ignoring"));
        return;
    }

    /* Writes to the bds characteristic will be ignored if the flow control bit is set */
    if(!_flush_bin_buf && !(_dfu_control & DFU_CTRL_FC_PAUSE_BIT)) {

        /* Check sequence number and make sure it's what we expected */
        if(seq_id == _seq_id) {
            /* 7-bit sequence ID rolls over at 127 */
            _seq_id++;
            _seq_id &= 0x7F;
            _bin_stream_buf.push(data.subspan(1));
            if(_bin_stream_buf.size() >= MBED_CONF_BLE_DFU_SERVICE_RX_FC_PAUSE_THRESHOLD) {
                set_fc_bit();
            }
            schedule_write();
        } else {
            /* Otherwise, notify the client that the expected sequence ID did not match using the status characteristic */
            TRACE_IF(tr_warn("sequence number does not match; expected: %d, actual: %d", _seq_id, seq_id));
            set_status(DFU_STATE_SYNC_LOSS_BIT | _seq_id);
        }
    }
}

void DFUService::on_dfu_ctrl_write_request(
        GattWriteAuthCallbackParams *params) {

    ControlChange change(*this, *params->data);

    if(change.get_changed_bits() & DFU_CTRL_READONLY_BITS) {
        /* Reject writes that modify read-only bits */
        TRACE_IF(tr_debug("dfu_ctrl write request: rejected (read-only)"));
        params->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_APP_READONLY;
        return;
    }

    if(_ctrl_req_cb) {
        /* Forward request to the application */
        params->authorizationReply = _ctrl_req_cb(change);
        TRACE_IF(tr_debug("dfu_ctrl write request: accepted (by application)"));
    } else {
        /* If no application handler, accept by default */
        TRACE_IF(tr_debug("dfu_ctrl write request: accepted"));
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void DFUService::on_dfu_ctrl_written(uint8_t new_ctrl) {
    TRACE_IF(tr_debug("dfu_ctrl written: %d", new_ctrl));
    mbed::ScopedLock<PlatformMutex> lock(_mutex);
    ControlChange change(*this, new_ctrl);
    /* Call application handler for control updates, if available */
    if(_ctrl_update_cb) {
        _ctrl_update_cb(change);
    }

    if(change.get_changed_bits() & DFU_CTRL_ENABLE_BIT) {
        TRACE_IF(tr_debug("dfu mode %s", (change.value() & DFU_CTRL_ENABLE_BIT) ? "enabled" : "aborted"));

        if(change.value() & DFU_CTRL_ENABLE_BIT) {
            /* If DFU is being enabled, clear the currently-selected update slot */
            _queue.call(mbed::callback(this, &DFUService::init_selected_slot));
        }
    }

    if(change.get_changed_bits() & DFU_CTRL_DELTA_MODE_EN_BIT) {
        TRACE_IF(tr_debug("delta mode %s", (change.value() & DFU_CTRL_DELTA_MODE_EN_BIT) ? "enabled" : "disabled"));
    }

    if(change.get_changed_bits() & DFU_CTRL_COMMIT_BIT) {
        TRACE_IF(tr_debug("dfu commit"));
    }

    _dfu_control = new_ctrl;
}

void DFUService::init_selected_slot(void) {
    mbed::ScopedLock<PlatformMutex> lock(_mutex); // TODO mutex lock necessary here?
    TRACE_IF(tr_debug("initializing slot %d", _selected_slot))
    mbed::BlockDevice* slot = _slot_bds[_selected_slot];
    slot->init();
    slot->erase(0, slot->size());
    // Send a neutral notification of the status characteristic to tell the client we're ready
    set_status(DFU_STATE_IDLE);
}


void DFUService::process_buffer(void) {

    /* TODO Rework the whole writing buffered data stuff */

    mbed::BlockDevice* slot = _slot_bds[_selected_slot];
    /* Attempt to write as many multiples of program size as possible at once */

    /**
     * TODO is there a better way to do this? Without dynamic allocation, we would
     * have to program byte-by-byte. This would likely be a significant hit in speed.
     */
    bd_size_t write_size = (_bin_stream_buf.size() / slot->get_program_size()) * slot->get_program_size();
    TRACE_IF(tr_debug("processing buffer: %lu => %lu",
            _bin_stream_buf.size(),
            write_size));
    if(write_size == 0) {
        /* Skip 0-length writes */
        _scheduled_write = 0;
        return;
    }
    uint8_t *temp_buf = new uint8_t[write_size];
    _bin_stream_buf.pop(temp_buf, write_size);
    int result = slot->program(temp_buf, _current_offset, write_size);
    if(result) {
        TRACE_IF(tr_err("programming memory error: %d", result));
        set_status(DFU_STATE_FLASH_ERROR);
    }
    _current_offset += write_size;
    delete[] temp_buf;

    /* If the buffer isn't empty and a flush should be performed, pad the write */
    if(_bin_stream_buf.size() && _flush_bin_buf) {
        TRACE_IF(tr_debug("flushing buffer: %lu => %lu",
                _bin_stream_buf.size(),
                write_size));
        write_size = slot->get_program_size();
        temp_buf = new uint8_t[write_size];
        /* Pad the write buffer with the BD's erase value */
        memset(temp_buf, slot->get_erase_value(), write_size);
        /* Empty the buffer */
        _bin_stream_buf.pop(temp_buf, _bin_stream_buf.size());

        /* Flush to flash */
        result = slot->program(temp_buf, _current_offset, write_size);
        if(result) {
            set_status(DFU_STATE_FLASH_ERROR);
        }
        _current_offset += write_size;
        delete[] temp_buf;
        flush_complete();
    }

    _scheduled_write = 0;
    schedule_write();

}


#endif //BLE_FEATURE_GATT_SERVER

