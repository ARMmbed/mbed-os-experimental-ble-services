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

/* Disconnection Service
 *
 * The disconnection service immediately disconnects from the client if a valid disconnections reason
 * is written to the Disconnection Reason characteristic. The valid reasons are:
 *
 * AUTHENTICATION_FAILURE                      = 0x05
 * CONNECTION_TIMEOUT                          = 0x08
 * REMOTE_USER_TERMINATED_CONNECTION           = 0x13
 * REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES = 0x14
 * REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF     = 0x15
 * LOCAL_HOST_TERMINATED_CONNECTION            = 0x16
 * UNACCEPTABLE_CONNECTION_PARAMETERS          = 0x3B
 *
 * If a valid disconnection reason is written to the Disconnection Reason characteristic,
 * The service triggers a local disconnection, resulting in the disconnection callback being called.
 * Therefore, the disconnection reason stored in the event will be LOCAL_HOST_TERMINATED_CONNECTION (0x16).
 * The service is added to the chain of gap event handlers during the initialization process.
 * As such, the disconnection event is registered in the service, which changes the disconnection reason
 * to the value written by the client. For example, if the client wrote CONNECTION_TIMEOUT (0x13)
 * to the Disconnection Reason characteristic, the disconnection reason stored in the event will now be
 * CONNECTION_TIMEOUT (0x13).
 */
class DisconnectionService : private ble::Gap::EventHandler,
                             private mbed::NonCopyable<DisconnectionService> {
public:
    const char *UUID_DISCONNECTION_SERVICE     = "f4361e6e-779d-11eb-9439-0242ac130002";
    const char *UUID_DISCONNECTION_REASON_CHAR = "f43620d0-779d-11eb-9439-0242ac130002";

    DisconnectionService(BLE &ble, ChainableGapEventHandler &chainable_gap_event_handler);

    ~DisconnectionService() = default;

    ble_error_t init();

    ChainableGapEventHandler& get_chainable_gap_event_handler_proxy();

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