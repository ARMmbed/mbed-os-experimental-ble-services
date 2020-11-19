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

#ifndef MBED_OS_EXPERIMENTAL_BLE_SERVICES_DESCRIPTORS_CHARACTERISTICUSERDESCRIPTIONDESCRIPTOR_H_
#define MBED_OS_EXPERIMENTAL_BLE_SERVICES_DESCRIPTORS_CHARACTERISTICUSERDESCRIPTIONDESCRIPTOR_H_

#include "ble/gatt/GattCharacteristic.h"
#include "ble/gatt/GattAttribute.h"

/**
 * Class encapsulating a Characteristic User Description Descriptor (CUDD)
 *
 * See Bluetooth Core Specification 5.2, Volume 3, Part G, Section 3.3.3.2
 *
 * TODO extend this in the future to support client-writable CUDD (set Writable Auxiliary bit)
 */
class CharacteristicUserDescriptionDescriptor : public GattAttribute
{
public:

    CharacteristicUserDescriptionDescriptor(const char* user_description) :
        GattAttribute((const UUID&) UUID(BLE_UUID_DESCRIPTOR_CHAR_USER_DESC),
        (uint8_t*) user_description,
        (user_description != nullptr) ? strlen(user_description) : 0,
        (user_description != nullptr) ? strlen(user_description) : 0,
        false),
        _user_description(user_description)
    {
        this->allowWrite(false);
    }

    const char* get_user_description() const {
        return _user_description;
    }

private:

    const char* _user_description;


};

#endif /* MBED_OS_EXPERIMENTAL_BLE_SERVICES_DESCRIPTORS_CHARACTERISTICUSERDESCRIPTIONDESCRIPTOR_H_ */
