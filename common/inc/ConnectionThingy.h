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

#ifndef MBED_OS_EXPERIMENTAL_BLE_SERVICES_COMMON_INC_CONNECTIONTHINGY_H_
#define MBED_OS_EXPERIMENTAL_BLE_SERVICES_COMMON_INC_CONNECTIONTHINGY_H_

/**
 * Connection... Thingy?
 * A common problem encountered when developing BLE applications is:
 *
 * Characteristic read/write/notify/indicate events are handled in batch.
 *
 * This creates a lot of boilerplate code where a service must filter out by comparing
 * which handle was written to the callback's parameters, figure out which connection,
 * and then perform the appropriate action.
 *
 * "Global (modify this jargon?)" services are those that do not change behavior (ie: don't care)
 * about what connection handle is interacting with the characteristic. The data going in and out
 * is treated the same as if each connection handle was the same.
 *
 * The proposed API introduces a "ConnectionOrientedGattCharacteristic" (working name) that encapsulates
 * the logic required to create gatt characteristic event handlers that are instantiated with an associated
 * connection handle and share a lifetime with that connection. When a connection is terminated, these event handlers
 * are not invalidated or nullified. Doing so would burden application code with the responsibility of keeping track of
 * connection/disconnection events so as not to access a "ConnectionOrientedGattCharacteristic" with an invalid connection handle.
 * Instead, the "ConnectionOrientedGattCharacteristic" handles are provided to the application as "SharedPtr" instances. This allows
 * the "ConnectionOrientedGattCharacteristic" instance to remain valid, lets the service with the characteristic remove its reference,
 * and prevents the application from encountering unexpected null pointers when attempting to use a "ConnectionOrientedGattCharacteristic"
 * that has been disconnected.
 *
 * In addition, the logic required to create Characteristic-specific read/write handlers should be added to the BLE API or as an extension.
 * Proposed APIs:
 * GattCharacteristic::EventHandler::onWritten
 * GattCharacteristic::EventHandler::onRead
 * GattCharacteristic::... and so on
 *
 * Maybe connection oriented service? Service logic
 *
 *
 */




#endif /* MBED_OS_EXPERIMENTAL_BLE_SERVICES_COMMON_INC_CONNECTIONTHINGY_H_ */
