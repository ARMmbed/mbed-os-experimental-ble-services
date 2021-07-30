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

#if BLE_FEATURE_GATT_SERVER

#include "ble-service-fota/FOTAService.h"

#include "ble/gatt/GattCharacteristic.h"

#include "mbed-trace/mbed_trace.h"

#include "platform/mbed_assert.h"
#include "platform/ScopedLock.h"

#define TRACE_GROUP "FOTA"

namespace uuids {
namespace FOTAService {

     const char BaseUUID[]          = "64121000-8b71-4181-5f43-08de72451679";
     const char BinaryStreamUUID[]  = "64122001-8b71-4181-5f43-08de72451679";
     const char ControlUUID[]       = "64122000-8b71-4181-5f43-08de72451679";
     const char StatusUUID[]        = "64122002-8b71-4181-5f43-08de72451679";
     const char VersionUUID[]       = "64122003-8b71-4181-5f43-08de72451679";

}}

FOTAService::FOTAService(BLE &ble, events::EventQueue &event_queue, ChainableGapEventHandler &chainable_gap_eh,
        ChainableGattServerEventHandler &chainable_gatt_server_eh, const char *protocol_version,
        const char *fw_rev, const char *dev_desc) : _ble(ble), _event_queue(event_queue),
                _chainable_gap_eh(chainable_gap_eh), _chainable_gatt_server_eh(chainable_gatt_server_eh),
                _protocol_version_str(protocol_version), _fw_rev_str(fw_rev), _fw_cudd(dev_desc),
                _binary_stream_char(uuids::FOTAService::BinaryStreamUUID, (uint8_t*) _rxbuf, 2, BLE_FOTA_SERVICE_MAX_DATA_LEN,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE),
                        nullptr, 0, true),
                _ctrl_char(uuids::FOTAService::ControlUUID, _control, 1, MBED_CONF_BLE_SERVICE_FOTA_CONTROL_BUFFER_SIZE,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
                        nullptr, 0, true),
                _status_char(uuids::FOTAService::StatusUUID, _status, 1, MBED_CONF_BLE_SERVICE_FOTA_STATUS_BUFFER_SIZE,
                        (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
                         nullptr, 0, true),
                _protocol_version_char(uuids::FOTAService::VersionUUID, (uint8_t*)_protocol_version_str,
                        strlen(_protocol_version_str), strlen(_protocol_version_str),
                        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                        nullptr, 0, false),
                /* FW Rev Char will be dropped by GattServer if val in nullptr, len is 0, and it's readable */
                _firmware_rev_char(GattCharacteristic::UUID_FIRMWARE_REVISION_STRING_CHAR,
                        (uint8_t *) fw_rev,
                        (fw_rev != nullptr) ? strlen(fw_rev) : 0, /* Min length */
                        (fw_rev != nullptr) ? strlen(fw_rev) : 0, /* Max length */
                        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ,
                        _fw_descs,
                        (dev_desc != nullptr) ? 1 : 0,
                        true){
}

FOTAService::~FOTAService() {
}

ble_error_t FOTAService::init() {

    GattCharacteristic *char_table[] = {
            &_binary_stream_char,
            &_ctrl_char,
            &_status_char,
            &_protocol_version_char,
            /* Note: FW rev must always be at the end of the characteristics list since it is optional */
            &_firmware_rev_char
    };

    GattService fota_service(uuids::FOTAService::BaseUUID, char_table,
            (_fw_rev_str != nullptr) ?
            (sizeof(char_table) / sizeof(char_table[0])) :      /** FW rev present */
            (sizeof(char_table) / sizeof(char_table[0])) - 1);  /** FW rev absent */


    /** Setup write authorization callbacks */
    _ctrl_char.setWriteAuthorizationCallback(this, &FOTAService::on_control_write_request);

    /** Note: Characteristic authorization callbacks must be set BEFORE adding the service! */
    ble_error_t error = _ble.gattServer().addService(fota_service);

    if(error == BLE_ERROR_NONE) {
        _chainable_gap_eh.addEventHandler(this);
        _chainable_gatt_server_eh.addEventHandler(this);
    } else {
        tr_error("error occurred when registering FOTA Service: %d", error);
    }

    return error;
}

void FOTAService::set_event_handler(EventHandler *handler) {
    _eh = handler;
}

void FOTAService::set_xoff() {
    _flow_paused = true;
    const uint8_t status_update[2] = { FOTA_STATUS_XOFF, _fragment_id };
    notify_status(mbed::make_const_Span(status_update));
}

void FOTAService::set_xon() {
    _flow_paused = false;
    const uint8_t status_update[2] = { FOTA_STATUS_XON, _fragment_id };
    notify_status(mbed::make_const_Span(status_update));
}

void FOTAService::notify_status(mbed::Span<const uint8_t> buf) {
    _ble.gattServer().write(_status_char.getValueHandle(), buf.data(), buf.size());
}

void FOTAService::notify_status(uint8_t code) {
    notify_status(mbed::make_const_Span(&code, 1));
}

void FOTAService::reset(void) {
    _fota_in_session = false;
    _fragment_id = 0;
    _flow_paused = false;
    _sync_lost = false;
}

void FOTAService::on_control_write_request(
        GattWriteAuthCallbackParams *write_request) {

    /* Check if there's an ongoing FOTA session. Reject COMMIT and STOP if no session */
    if(!_fota_in_session) {
        if((write_request->data[0] == FOTA_COMMIT) || (write_request->data[0] == FOTA_STOP)) {
            write_request->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_REQUEST_REJECTED;
            return;
        }
    }

    /* Check if there's not an ongoing FOTA session. Reject START if in session */
    if(_fota_in_session) {
        if(write_request->data[0] == FOTA_START) {
            write_request->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_REQUEST_REJECTED;
            return;
        }
    }

    /* Check if we're in sync (unless it's a STOP command) */
    if(_sync_lost && (write_request->data[0] != FOTA_STOP)) {
        write_request->authorizationReply = (GattAuthCallbackReply_t) AUTH_CALLBACK_REPLY_ATTERR_OUT_OF_SYNC;
        notify_sync_lost();
        return;
    }

    if(_eh) {
        write_request->authorizationReply = _eh->on_control_written(*this, mbed::make_const_Span(write_request->data, write_request->len));
    }

}

void FOTAService::onDisconnectionComplete(
        const ble::DisconnectionCompleteEvent &event) {
    reset();
}

void FOTAService::onDataWritten(const GattWriteCallbackParams &params) {
    if(params.handle == _binary_stream_char.getValueHandle()) {
        on_bsc_written(mbed::make_const_Span(params.data, params.len));
    }

    /* Writes to the Control characteristic are handled in the auth callback */
}

void FOTAService::on_bsc_written(mbed::Span<const uint8_t> data) {

    /* Check if there's a FOTA session in progress */
    if(!_fota_in_session) {
        uint8_t status = FOTA_STATUS_NO_FOTA_SESSION;
        notify_status(mbed::make_const_Span(&status, 1));
        return;
    }

    /* Now check if flow is paused */
    if(_flow_paused) {
        /* Resend the XOFF notification */
        set_xoff();
        return;
    }

    /* Now check the fragment ID */
    if(data[0] != (_fragment_id)) {
        tr_warn("received fragment id %d, expected %d", data[0], _fragment_id);
        /* Issue SYNC_LOST notification */
        _sync_lost = true;
        notify_sync_lost();
        return;
    } else {
        _sync_lost = false;
    }

    StatusCode_t result = FOTA_STATUS_OK;

    if(_eh) {
        /* The EventHandler implementation may notify status as appropriate in this call */
        result = _eh->on_binary_stream_written(*this, data.subspan(1));
    }

    if(result == FOTA_STATUS_OK) {
        _fragment_id++;
    }
}

void FOTAService::notify_sync_lost() {
    uint8_t status[] = { FOTA_STATUS_SYNC_LOST, _fragment_id };
    notify_status(mbed::make_const_Span(status));
}

void FOTAService::start_fota_session(void) {
    _fota_in_session = true;
}

void FOTAService::stop_fota_session(void) {
    _fota_in_session = false;
}

#endif //BLE_FEATURE_GATT_SERVER

