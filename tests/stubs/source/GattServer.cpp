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

#include "ble/GattServer.h"
#include "ble/gatt/GattAttribute.h"

namespace ble {

namespace {
static constexpr const unsigned int NOTIFY_PROPERTY =
    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY;
static constexpr const  unsigned int INDICATE_PROPERTY =
    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE;

static constexpr const  uint8_t UPDATE_PROPERTIES =
    NOTIFY_PROPERTY |
    INDICATE_PROPERTY;
}

void GattServer::setEventHandler(EventHandler *handler)
{
    _handler = handler;
}

GattServer::EventHandler *GattServer::getEventHandler()
{
    return _handler;
}

ble_error_t GattServer::addService(GattService &service)
{
    // This function fill handles like a real service would, it doesn't verify
    // that characteristics have been correctly declared.
    service.setHandle(++_last_attribute);
    for (size_t i = 0; i < service.getCharacteristicCount(); ++i) {
        auto *characteristic = service.getCharacteristic(i);
        ++_last_attribute; // skip the declaration attribute
        characteristic->getValueAttribute().setHandle(++_last_attribute);

        bool cccd_added = false;
        for (size_t j = 0; j < characteristic->getDescriptorCount(); ++j) {
            auto *descriptor = characteristic->getDescriptor(j);
            descriptor->setHandle(++_last_attribute);
            if (descriptor->getUUID() == UUID(BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)) {
                cccd_added = true;
            }
        }

        // Add the implicit CCCD if required
        if (!cccd_added && characteristic->getProperties() & UPDATE_PROPERTIES) {
            auto *cccd_value = new uint16_t {0};

            GattAttribute* implicit_cccd = new GattAttribute {
                UUID(BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG),
                reinterpret_cast<uint8_t*>(cccd_value),
                sizeof(*cccd_value),
                sizeof(*cccd_value),
                false
            };

            implicit_cccd->setHandle(++_last_attribute);
            implicit_cccd->allowRead(true);
            implicit_cccd->allowWrite(true);
            characteristic->setImplicitCCCD(implicit_cccd);

            // store the CCCD value so they are destroyed when the server is
            // deleted. The attributes are handled by the characteristic.
            _cccd_values.emplace_back(cccd_value);
        }
    }

    return BLE_ERROR_NONE;
}


}
