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

#ifndef DISCONNECTION_SERVICE_H
#define DISCONNECTION_SERVICE_H

#include "ble/BLE.h"

#if BLE_FEATURE_GATT_SERVER

#include "ble/Gap.h"
#include "ble/gap/ChainableGapEventHandler.h"

class DisconnectionService : private ble::Gap::EventHandler,
                             private mbed::NonCopyable<DisconnectionService> {
public:
    const char *UUID_DISCONNECTION_SERVICE     = "f4361e6e-779d-11eb-9439-0242ac130002";
    const char *UUID_DISCONNECTION_REASON_CHAR = "f43620d0-779d-11eb-9439-0242ac130002";

    DisconnectionService(BLE &ble, ChainableGapEventHandler &chainable_gap_event_handler);

    ~DisconnectionService() = default;

    ble_error_t init();

    ChainableGapEventHandler& get_chainable_gap_event_handler_proxy() {
        return _chainable_gap_event_handler_proxy;
    }

private:
    void onDataWritten(GattWriteAuthCallbackParams *write_request);

    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

    BLE &_ble;
    ChainableGapEventHandler &_chainable_gap_event_handler;
    ChainableGapEventHandler  _chainable_gap_event_handler_proxy;

    bool _use_local_disconnection_reason = false;
    ble::disconnection_reason_t _disconnection_reason = ble::disconnection_reason_t::AUTHENTICATION_FAILURE;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif // DISCONNECTION_SERVICE_H