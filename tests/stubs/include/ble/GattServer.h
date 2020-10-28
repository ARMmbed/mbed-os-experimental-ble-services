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


#ifndef MBED_GATT_SERVER_H__
#define MBED_GATT_SERVER_H__

#include <vector>
#include <memory>

#include "platform/mbed_toolchain.h"

#include "ble/common/CallChainOfFunctionPointersWithContext.h"
#include "ble/common/blecommon.h"

#include "ble/gatt/GattService.h"
#include "ble/gatt/GattAttribute.h"
#include "ble/gatt/GattCallbackParamTypes.h"

namespace ble {

class GattServer {
public:
    /**
     * Definition of the general handler of GattServer related events.
     */
    struct EventHandler {
        /**
         * Function invoked when the connections changes the ATT_MTU which controls
         * the maximum size of an attribute that can be read in a single L2CAP packet
         * which might be fragmented across multiple packets.
         *
         * @param connectionHandle The handle of the connection that changed the size.
         * @param attMtuSize
         */
        virtual void onAttMtuChange(
            ble::connection_handle_t connectionHandle,
            uint16_t attMtuSize
        ) {
            (void)connectionHandle;
            (void)attMtuSize;
        }

        /**
         * Function invoked when the server has sent data to a client as
         * part of a notification/indication.
         *
         * @note params has a temporary scope and should be copied by the
         * application if needed later
         */
        virtual void onDataSent(const GattDataSentCallbackParams &params) {
            (void)params;
        }

        /**
         * Function invoked when a client writes an attribute
         *
         * @note params has a temporary scope and should be copied by the
         * application if needed later
         */
        virtual void onDataWritten(const GattWriteCallbackParams &params) {
            (void)params;
        }

        /**
         * Function invoked when a client reads an attribute
         *
         * @note  This functionality may not be available on all underlying stacks.
         * Application code may work around that limitation by monitoring read
         * requests instead of read events.
         *
         * @note params has a temporary scope and should be copied by the
         * application if needed later
         *
         * @see GattCharacteristic::setReadAuthorizationCallback()
         * @see isOnDataReadAvailable().
         */
        virtual void onDataRead(const GattReadCallbackParams &params) {
            (void)params;
        }

        /**
         * Function invoked when the GattServer instance is about
         * to be shut down. This can result in a call to reset() or BLE::reset().
         */
        virtual void onShutdown(const GattServer &server) {
            (void)server;
        }

        /**
         * Function invoked when the client has subscribed to characteristic updates
         *
         * @note params has a temporary scope and should be copied by the
         * application if needed later
         */
        virtual void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) {
            (void)params;
        }

        /**
         * Function invoked when the client has unsubscribed to characteristic updates
         *
         * @note params has a temporary scope and should be copied by the
         * application if needed later
         */
        virtual void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) {
            (void)params;
        }

        /**
         * Function invoked when an ACK has been received for an
         * indication sent to the client.
         *
         * @note params has a temporary scope and should be copied by the
         * application if needed later
         */
        virtual void onConfirmationReceived(const GattConfirmationReceivedCallbackParams &params) {
            (void)params;
        }

    protected:
        /**
         * Prevent polymorphic deletion and avoid unnecessary virtual destructor
         * as the GattServer class will never delete the instance it contains.
         */
        ~EventHandler() = default;
    };


public:

    GattServer() = default;
    GattServer(const GattServer&) = delete;
    GattServer& operator=(const GattServer&) = delete;
    virtual ~GattServer() = default;

    /**
     * Assign the event handler implementation that will be used by the
     * module to signal events back to the application.
     *
     * @param handler Application implementation of an EventHandler.
     *
     * @note Multiple discrete EventHandler instances may be used by adding them
     * to a ChainableGattServerEventHandler and then setting the chain as the primary
     * GattServer EventHandler using this function.
     *
     * @see ChainableGattServerEventHandler
     */
    void setEventHandler(EventHandler *handler);

    EventHandler *getEventHandler();

    /**
     * Add a service declaration to the local attribute server table.
     *
     * This functions inserts a service declaration in the attribute table
     * followed by the characteristic declarations (including characteristic
     * descriptors) present in @p service.
     *
     * The process assigns a unique attribute handle to all the elements added
     * into the attribute table. This handle is an ID that must be used for
     * subsequent interractions with the elements.
     *
     * @note There is no mirror function that removes a single service.
     * Application code can remove all the registered services by calling
     * reset().
     *
     * @attention Service, characteristics and descriptors objects registered
     * within the GattServer must remain reachable until reset() is called.
     *
     * @param[in] service The service to be added; attribute handle of services,
     * characteristic and characteristic descriptors are updated by the
     * process.
     *
     * @return BLE_ERROR_NONE if the service was successfully added.
     */
    virtual ble_error_t addService(GattService &service);

    /**
     * Read the value of an attribute present in the local GATT server.
     *
     * @param[in] attributeHandle Handle of the attribute to read.
     * @param[out] buffer A buffer to hold the value being read.
     * @param[in,out] lengthP Length of the buffer being supplied. If the
     * attribute value is longer than the size of the supplied buffer, this
     * variable holds upon return the total attribute value length (excluding
     * offset). The application may use this information to allocate a suitable
     * buffer size.
     *
     * @return BLE_ERROR_NONE if a value was read successfully into the buffer.
     *
     * @attention read(ble::connection_handle_t, GattAttribute::Handle_t, uint8_t *, uint16_t *)
     * must be used to read Client Characteristic Configuration Descriptor (CCCD)
     * because the value of this type of attribute depends on the connection.
     */
    virtual ble_error_t read(
        GattAttribute::Handle_t attributeHandle,
        uint8_t buffer[],
        uint16_t *lengthP
    ) = 0;

    /**
     * Read the value of an attribute present in the local GATT server.
     *
     * The connection handle allows application code to read the value of a
     * Client Characteristic Configuration Descriptor for a given connection.
     *
     * @param[in] connectionHandle Connection handle.
     * @param[in] attributeHandle Attribute handle for the value attribute of
     * the characteristic.
     * @param[out] buffer A buffer to hold the value being read.
     * @param[in,out] lengthP Length of the buffer being supplied. If the
     * attribute value is longer than the size of the supplied buffer, this
     * variable holds upon return the total attribute value length (excluding
     * offset). The application may use this information to allocate a suitable
     * buffer size.
     *
     * @return BLE_ERROR_NONE if a value was read successfully into the buffer.
     */
    virtual ble_error_t read(
        ble::connection_handle_t connectionHandle,
        GattAttribute::Handle_t attributeHandle,
        uint8_t *buffer,
        uint16_t *lengthP
    ) = 0;

    /**
     * Update the value of an attribute present in the local GATT server.
     *
     * @param[in] attributeHandle Handle of the attribute to write.
     * @param[in] value A pointer to a buffer holding the new value.
     * @param[in] size Size in bytes of the new value (in bytes).
     * @param[in] localOnly If this flag is false and the attribute handle
     * written is a characteristic value, then the server sends an update
     * containing the new value to all clients that have subscribed to the
     * characteristic's notifications or indications. Otherwise, the update does
     * not generate a single server initiated event.
     *
     * @return BLE_ERROR_NONE if the attribute value has been successfully
     * updated.
     */
    virtual ble_error_t write(
        GattAttribute::Handle_t attributeHandle,
        const uint8_t *value,
        uint16_t size,
        bool localOnly = false
    ) = 0;

    /**
     * Update the value of an attribute present in the local GATT server.
     *
     * The connection handle parameter allows application code to direct
     * notification or indication resulting from the update to a specific client.
     *
     * @param[in] connectionHandle Connection handle.
     * @param[in] attributeHandle Handle for the value attribute of the
     * characteristic.
     * @param[in] value A pointer to a buffer holding the new value.
     * @param[in] size Size of the new value (in bytes).
     * @param[in] localOnly If this flag is false and the attribute handle
     * written is a characteristic value, then the server sends an update
     * containing the new value to the client identified by the parameter
     * @p connectionHandle if it is subscribed to the characteristic's
     * notifications or indications. Otherwise, the update does not generate a
     * single server initiated event.
     *
     * @return BLE_ERROR_NONE if the attribute value has been successfully
     * updated.
     */
    virtual ble_error_t write(
        ble::connection_handle_t connectionHandle,
        GattAttribute::Handle_t attributeHandle,
        const uint8_t *value,
        uint16_t size,
        bool localOnly = false
    ) = 0;

    /**
     * Determine if one of the connected clients has subscribed to notifications
     * or indications of the characteristic in input.
     *
     * @param[in] characteristic The characteristic.
     * @param[out] enabledP Upon return, *enabledP is true if updates are
     * enabled for a connected client; otherwise, *enabledP is false.
     *
     * @return BLE_ERROR_NONE if the connection and handle are found. False
     * otherwise.
     */
    virtual ble_error_t areUpdatesEnabled(
        const GattCharacteristic &characteristic,
        bool *enabledP
    ) = 0;

    /**
     * Determine if an identified client has subscribed to notifications or
     * indications of a given characteristic.
     *
     * @param[in] connectionHandle The connection handle.
     * @param[in] characteristic The characteristic.
     * @param[out] enabledP Upon return, *enabledP is true if the client
     * identified by @p connectionHandle has subscribed to notifications or
     * indications of @p characteristic; otherwise, *enabledP is false.
     *
     * @return BLE_ERROR_NONE if the connection and handle are found. False
     * otherwise.
     */
    virtual ble_error_t areUpdatesEnabled(
        ble::connection_handle_t connectionHandle,
        const GattCharacteristic &characteristic,
        bool *enabledP
    ) = 0;

    /**
     * Indicate if the underlying stack emit events when an attribute is read by
     * a client.
     *
     * @attention This function should be overridden to return true if
     * applicable.
     *
     * @return true if onDataRead is supported; false otherwise.
     */
    virtual bool isOnDataReadAvailable() const = 0;

private:
    EventHandler *_handler = nullptr;
    uint16_t _last_attribute = 0;
    std::vector<std::unique_ptr<uint16_t>> _cccd_values;
};

/**
 * @}
 * @}
 * @}
 */

} // ble

/** @deprecated Use the namespaced ble::GattServer instead of the global GattServer. */
using ble::GattServer;

#endif /* ifndef MBED_GATT_SERVER_H__ */
