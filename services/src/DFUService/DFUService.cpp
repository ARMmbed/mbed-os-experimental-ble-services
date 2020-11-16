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

#ifndef MBED_CONF_BLE_DFU_SERVICE_TRACE_ENABLE
#define MBED_CONF_BLE_DFU_SERVICE_TRACE_ENABLE 0
#endif

#if MBED_CONF_BLE_DFU_SERVICE_TRACE_ENABLE
#define TRACE_IF(x) do { x } while(0);
#else
#define TRACE_IF(x)
#endif

#if BLE_FEATURE_GATT_SERVER

#include "DFUService.h"

#include "ble/gatt/GattCharacteristic.h"

#include "mbed_trace.h"

#include "platform/mbed_assert.h"
#include "platform/ScopedLock.h"

DFUService::DFUService(mbed::BlockDevice* bd) :
    _slot_char(uuids::DFUService::SlotUUID, &_selected_slot, 1, 1,
            (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
             nullptr, 0, false),
    _offset_char(uuids::DFUService::OffsetUUID, (uint8_t*) &_current_offset, 4, 4,
            (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
             nullptr, 0, false),
    _rx_char(uuids::DFUService::BinaryStreamUUID, _rxbuf,
            1, BLE_DFU_SERVICE_MAX_DATA_LEN,
            (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE)),
    _dfu_ctrl_char(uuids::DFUService::ControlUUID, &_dfu_control, 1, 1,
            (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE),
             nullptr, 0, false),
    _status_char(uuids::DFUService::StatusUUID, &_status, 1, 1,
            (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY &
             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE),
             nullptr, 0, false),
    _dfu_service(uuids::DFUService::BaseUUID,
            _characteristics,
            sizeof(_characteristics)/
            sizeof(_characteristics[0])),
    _server(nullptr)
{

    _slot_bds[0] = bd;

    _characteristics[0] = &_slot_char;
    _characteristics[1] = &_offset_char;
    _characteristics[2] = &_rx_char;
    _characteristics[3] = &_dfu_ctrl_char;
    _characteristics[4] = &_status_char;

}

DFUService::~DFUService() {
}

void DFUService::start(BLE &ble_interface) {
    _server = &ble_interface.gattServer();
    _server->addService(_dfu_service);

    /** Setup write authorization callbacks */
    _slot_char.setWriteAuthorizationCallback(this, &DFUService::on_slot_write_request);
    _offset_char.setWriteAuthorizationCallback(this, &DFUService::on_offset_write_request);
    _dfu_ctrl_char.setWriteAuthorizationCallback(this, &DFUService::on_dfu_ctrl_write_request);
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
        // TODO verify dereferencing works here (endianness)
        on_offset_written((uint32_t)*params.data);
    } else
    if(params.handle == _rx_char.getValueHandle()) {
        on_bds_written(mbed::make_const_Span(params.data, params.len));
    } else
    if(params.handle == _dfu_ctrl_char.getValueHandle()) {
        on_dfu_ctrl_written(*params.data);
    }
}

void DFUService::set_status() {
}

void DFUService::onDisconnectionComplete(
        const ble::DisconnectionCompleteEvent &event) {
    // TODO determine disconnection behavior in various states
}

void DFUService::on_slot_write_request(GattWriteAuthCallbackParams *params) {
    /* Verify if desired slot is valid */
    uint8_t desired_slot = *params->data;
    if(!(desired_slot < MBED_CONF_BLE_DFU_SERVICE_MAX_SLOTS)) {
        params->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_APP_INVALID_SLOT_NUM;
    } else {
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void DFUService::on_slot_written(uint8_t new_slot) {
    // TODO flush data in rx buffer before switching slot
    mbed::ScopedLock<PlatformMutex> lock(_mutex);
    _slot_bds[_selected_slot]->deinit();
    _selected_slot = new_slot;
    _slot_bds[_selected_slot]->init();
}

void DFUService::on_offset_write_request(GattWriteAuthCallbackParams *params) {
    if(!_bin_stream_buf.empty()) {
        /* Reject offset write request and initiate a flush of the binary stream buffer */
        params->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_APP_BUSY;
        mbed::ScopedLock<PlatformMutex> lock(_mutex);
        _flush_bin_buf = true;
    } else {
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void DFUService::on_offset_written(uint32_t new_offset) {
    mbed::ScopedLock<PlatformMutex> lock(_mutex);
    _current_offset = new_offset;
}

void DFUService::on_bds_written(mbed::Span<const uint8_t> data) {
    /* Writes to the bds characteristic will be ignored if a buffer flush is in progress */
    if(!_flush_bin_buf) {
        _bin_stream_buf.push(data);
        // TODO if threshold exceeded, write flow control bit
    }
}

void DFUService::on_dfu_ctrl_write_request(
        GattWriteAuthCallbackParams *params) {
    if(_ctrl_req_cb) {
        /* Forward request to the application */
        ControlChange change(*this, *params->data);
        params->authorizationReply = _ctrl_req_cb(change);
    } else {
        /* If no application handler, accept by default */
        params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }
}

void DFUService::on_dfu_ctrl_written(uint8_t new_ctrl) {
    mbed::ScopedLock<PlatformMutex> lock(_mutex);
    /* Call application handler for control updates, if available */
    if(_ctrl_update_cb) {
        ControlChange change(*this, new_ctrl);
        _ctrl_update_cb(change);
    }
    _dfu_control = new_ctrl;
}

#endif //BLE_FEATURE_GATT_SERVER

