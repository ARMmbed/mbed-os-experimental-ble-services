/* mbed Microcontroller Library
 * Copyright (c) 2006-2020 ARM Limited
 *
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

#ifndef MBED_BLE_H__
#define MBED_BLE_H__

#include "platform/mbed_assert.h"
#include "platform/mbed_toolchain.h"

#include "ble/GattServer.h"

#include "ble/common/BLETypes.h"
#include "ble/common/blecommon.h"
#include "ble/common/FunctionPointerWithContext.h"

namespace ble {

class BLE {
public:
    // Prevent copy construction and copy assignment of BLE.
    BLE(const BLE &) = delete;
    BLE &operator=(const BLE &) = delete;

    /**
     * Get a reference to the BLE singleton.
     */
    static BLE &Instance();

    /**
     * Accessor to GattServer. All GattServer related functionality requires
     * going through this accessor.
     */
    ble::GattServer &gattServer();

    /**
     * A const alternative to gattServer().
     */
    const ble::GattServer &gattServer() const;

    /**
     * Set the gatt server that is returned by gattServer(). The caller manages
     * the lifetime of the instance provided.
     */
    void setGattServer(ble::GattServer *server);


private:
    BLE();

    ble::GattServer *_gatt_server = nullptr;
};

}

using ble::BLE;
/**
 * @namespace ble Entry namespace for all BLE API definitions.
 */

/**
 * @}
 */

#endif /* ifndef MBED_BLE_H__ */
