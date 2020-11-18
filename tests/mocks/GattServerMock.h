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

#ifndef BLE_SERVICES_GATTSERVERMOCK_H
#define BLE_SERVICES_GATTSERVERMOCK_H

#include "gmock/gmock.h"
#include "ble/GattServer.h"

class GattServerMock : public ble::GattServer {
public:

    GattServerMock();
    GattServerMock(const GattServerMock&) = delete;
    GattServerMock& operator=(const GattServerMock&) = delete;
    virtual ~GattServerMock();

    MOCK_METHOD(ble_error_t, addService, (GattService &service), (override));

    MOCK_METHOD(ble_error_t, read, (GattAttribute::Handle_t attributeHandle, uint8_t buffer[], uint16_t *lengthP), (override));

    MOCK_METHOD(ble_error_t, read, (ble::connection_handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, uint8_t buffer[], uint16_t *lengthP), (override));

    MOCK_METHOD(ble_error_t, write, (GattAttribute::Handle_t attributeHandle, const uint8_t *value, uint16_t size, bool localOnly), (override));

    MOCK_METHOD(ble_error_t, write, (ble::connection_handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, const uint8_t *value, uint16_t size, bool localOnly), (override));

    MOCK_METHOD(ble_error_t, areUpdatesEnabled, (const GattCharacteristic &characteristic, bool *enabledP), (override));

    MOCK_METHOD(ble_error_t, areUpdatesEnabled, (ble::connection_handle_t connectionHandle, const GattCharacteristic &characteristic, bool *enabledP), (override));

    MOCK_METHOD(bool, isOnDataReadAvailable, (), (const));
};



#endif //BLE_SERVICES_GATTSERVERMOCK_H