/* mbed Microcontroller Library
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
 * limitations under the License.
 */

#include "DisconnectionService.h"

DisconnectionService::DisconnectionService(BLE &ble, ChainableGapEventHandler &chainable_gap_event_handler) :
    _ble(ble),
    _chainable_gap_event_handler(chainable_gap_event_handler)
{
}

ble_error_t DisconnectionService::init()
{
    ReadWriteGattCharacteristic<ble::disconnection_reason_t> disconnection_reason_char(UUID_DISCONNECTION_REASON_CHAR, &_disconnection_reason);

    GattCharacteristic *charTable[] = { &disconnection_reason_char };
    GattService         disconnectionService(UUID_DISCONNECTION_SERVICE, charTable, 1);

    disconnection_reason_char.setWriteAuthorizationCallback(this, &DisconnectionService::onDataWritten);

    ble_error_t error = _ble.gattServer().addService(disconnectionService);

    if (error == BLE_ERROR_NONE) {
        _chainable_gap_event_handler.addEventHandler(this);
    }

    return error;
}

void DisconnectionService::onDataWritten(GattWriteAuthCallbackParams *write_request)
{
    _disconnection_reason = static_cast<ble::disconnection_reason_t>(*write_request->data);
    ble::connection_handle_t connectionHandle = write_request->connHandle;

    _use_local_disconnection_reason = true;
    _ble.gap().disconnect(connectionHandle, ble::local_disconnection_reason_t::USER_TERMINATION);
}

void DisconnectionService::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    _chainable_gap_event_handler_proxy.onConnectionComplete(event);
}

void DisconnectionService::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    if (_use_local_disconnection_reason) {
        auto new_event = ble::DisconnectionCompleteEvent(event.getConnectionHandle(), _disconnection_reason);
        _use_local_disconnection_reason = false;
        _chainable_gap_event_handler_proxy.onDisconnectionComplete(new_event);
    } else {
        _chainable_gap_event_handler_proxy.onDisconnectionComplete(event);
    }
}
