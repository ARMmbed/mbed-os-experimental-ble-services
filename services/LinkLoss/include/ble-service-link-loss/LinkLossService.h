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

#ifndef LINK_LOSS_SERVICE_H
#define LINK_LOSS_SERVICE_H

#if BLE_FEATURE_GATT_SERVER

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "events/EventQueue.h"
#include "ble/gap/ChainableGapEventHandler.h"

#include <chrono>

/**
 * Link Loss Service
 *
 * @par purpose
 * The link loss service uses the Alert Level characteristic, as defined in
 * https://www.bluetooth.com/specifications/assigned-numbers/, to cause an alert
 * in the device when the link is lost.
 *
 * @par usage
 * The on_alert_requested() and on_alert_end() event handlers should be overridden
 * by your application. For example, in the former case, you could alert the user by
 * flashing lights, making noises, moving, etc.
 *
 * This service requires access to gap events. Please register a
 * ChainableGapEventHandler with Gap and pass it to this service.
 *
 * @note The specification for the link loss service can be found here:
 * https://www.bluetooth.com/specifications/gatt
 *
 * @attention The user should not instantiate more than a single link loss service
 */
class LinkLossService : private ble::Gap::EventHandler {
public:
    enum class AlertLevel : uint8_t {
        NO_ALERT    = 0,
        MILD_ALERT  = 1,
        HIGH_ALERT  = 2
    };

    struct EventHandler {
        /**
         * On alert requested
         *
         * This function is called if the client disconnects ungracefully
         *
         * @attention This is an abstract function and should be overridden by the user.
         */
        virtual void on_alert_requested(AlertLevel) { }
        /**
         * On alert end
         *
         * This function is called if the alert is stopped
         *
         * @attention This is an abstract function and should be overridden by the user.
         */
        virtual void on_alert_end() { }
    };

    /**
     * Constructor
     *
     * Initialize the internal BLE, EventQueue and ChainableGapEventHandler objects to @p ble, @p event_queue
     * and @p chainable_gap_event_handler, respectively. Furthermore, configure the alert level characteristic
     * with the appropriate UUID and initialize it to 0x00 ("No Alert").
     *
     * @param ble BLE object to host the link loss service
     * @param event_queue EventQueue object to configure events
     * @param chainable_gap_event_handler ChainableGapEventHandler object to register multiple Gap events
     *
     * @attention The Initializer must be called after instantiating a link loss service.
     */
    LinkLossService(BLE &ble, events::EventQueue &event_queue, ChainableGapEventHandler &chainable_gap_event_handler);

    /**
     * Destructor
     *
     * Cancel the pending event queue timeout
     */
    ~LinkLossService();

    LinkLossService(const LinkLossService&) = delete;
    LinkLossService &operator=(const LinkLossService&) = delete;

    /**
     * Initializer
     *
     * Set the onDataWritten() function as the write authorization callback for the alert level characteristic.
     * Add the link loss service to the BLE device and chain of GAP event handlers.
     */
    void init();

    /**
     * Set event handler
     *
     * @param handler EventHandler object to handle events raised by the link loss service
     */
    void set_event_handler(EventHandler* handler);

    /**
     * Set alert level
     *
     * @param level New alert level
     */
    void set_alert_level(AlertLevel level);

    /**
     * Set alert timeout
     *
     * @param timeout Event queue timeout measured in ms
     */
    void set_alert_timeout(std::chrono::milliseconds timeout);

    /**
     * Get alert level
     *
     * @return AlertLevel The current alert level
     */
    AlertLevel get_alert_level();

    /**
     * Stop alert
     *
     * Cancel the pending event queue timeout
     */
    void stop_alert();

private:
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

    void onDataWritten(GattWriteAuthCallbackParams *write_request);

    BLE &_ble;
    events::EventQueue &_event_queue;
    ChainableGapEventHandler &_chainable_gap_event_handler;

    ReadWriteGattCharacteristic<AlertLevel> _alert_level_char;
    AlertLevel _alert_level = AlertLevel::NO_ALERT;
    std::chrono::milliseconds _alert_timeout = std::chrono::milliseconds(0);
    EventHandler *_alert_handler = nullptr;

    bool _in_alert = false;
    int _event_queue_handle = 0;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif // LINK_LOSS_SERVICE_H
