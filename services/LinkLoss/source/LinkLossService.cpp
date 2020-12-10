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

#if BLE_FEATURE_GATT_SERVER

#include "ble-service-link-loss/LinkLossService.h"

LinkLossService::LinkLossService(BLE &ble, events::EventQueue &event_queue, ChainableGapEventHandler &chainable_gap_event_handler) :
    _ble(ble),
    _event_queue(event_queue),
    _chainable_gap_event_handler(chainable_gap_event_handler),
    _alert_level_char(GattCharacteristic::UUID_ALERT_LEVEL_CHAR, &_alert_level)
{
}

LinkLossService::~LinkLossService()
{
    _event_queue.cancel(_event_queue_handle);
}

void LinkLossService::init()
{
    GattCharacteristic *charTable[] = { &_alert_level_char };
    GattService         linkLossService(GattService::UUID_LINK_LOSS_SERVICE, charTable, 1);

    _alert_level_char.setWriteAuthorizationCallback(this, &LinkLossService::onDataWritten);

    ble_error_t error = _ble.gattServer().addService(linkLossService);

    if (error == BLE_ERROR_NONE) {
        _chainable_gap_event_handler.addEventHandler(this);
    }
}

void LinkLossService::set_event_handler(EventHandler* handler)
{
    _alert_handler = handler;
}

void LinkLossService::set_alert_level(AlertLevel level)
{
    _alert_level = level;
}

void LinkLossService::set_alert_timeout(std::chrono::milliseconds timeout)
{
    _alert_timeout = timeout;
}

LinkLossService::AlertLevel LinkLossService::get_alert_level()
{
    return _alert_level;
}

void LinkLossService::stop_alert()
{
    _event_queue.cancel(_event_queue_handle);
    _event_queue_handle = 0;
    if (_in_alert) {
        _in_alert = false;
        if (_alert_handler) {
            _alert_handler->on_alert_end();
        }
    }
}

void LinkLossService::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    if (event.getStatus() == BLE_ERROR_NONE) {
        stop_alert();
    }
}

void LinkLossService::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    AlertLevel level = get_alert_level();
    if (_alert_handler != nullptr && event.getReason() == ble::disconnection_reason_t::CONNECTION_TIMEOUT &&
        level != AlertLevel::NO_ALERT && !_in_alert) {
        _in_alert = true;
        _alert_handler->on_alert_requested(level);
        if (_alert_timeout > std::chrono::milliseconds(0)) {
            _event_queue_handle = _event_queue.call_in(_alert_timeout, [this] { stop_alert(); });
        }
    }
}

void LinkLossService::onDataWritten(GattWriteAuthCallbackParams *write_request)
{
    const uint8_t level = *write_request->data;

    if (level <= (uint8_t)(AlertLevel::HIGH_ALERT)) {
        set_alert_level((AlertLevel) level);
    } else {
        // The alert level is out of range
        write_request->authorizationReply = static_cast<GattAuthCallbackReply_t>(0xFF);
    }
}

#endif // BLE_FEATURE_GATT_SERVER
