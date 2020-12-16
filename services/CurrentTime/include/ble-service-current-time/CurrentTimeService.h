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

#ifndef CURRENT_TIME_SERVICE_H
#define CURRENT_TIME_SERVICE_H

#ifdef BLE_FEATURE_GATT_SERVER

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "events/EventQueue.h"
#include "mbed_rtc_time.h"

#include <ctime>

#define CURRENT_TIME_CHAR_VALUE_SIZE 10

/**
 * Current Time Service
 *
 * @par purpose
 * The current time service allows a BLE device to expose date and time information to other BLE devices.
 *
 * @par usage
 * The on_current_time_changed() event handler should overridden by your application
 *
 * @note The specification for the current time service can be found here:
 * https://www.bluetooth.com/specifications/gatt
 *
 * @attention The user should not instantiate more than a single current time service service
 */
class CurrentTimeService {
public:
    MBED_PACKED(struct) CurrentTime {
        /**
         * Year as defined by the Gregorian calendar.
         * Valid range 1582 to 9999.
         */
        uint16_t year;
        /**
         * Month of the year as defined by the Gregorian calendar.
         * Valid range 1 (January) to 12 (December).
         */
        uint8_t  month;
        /**
         * Day of the month as defined by the Gregorian calendar.
         * Valid range 1 to 31.
         */
        uint8_t  day;
        /**
         * Number of hours past midnight.
         * Valid range 0 to 23.
         */
        uint8_t  hours;
        /**
         * Number of minutes since the start of the hour.
         * Valid range 0 to 59.
         */
        uint8_t  minutes;
        /**
         * Number of seconds since the start of the minute.
         * Valid range 0 to 59.
         */
        uint8_t  seconds;
        /**
         * Days of a seven-day week as specified in ISO 8601.
         * Valid range from Monday (1) to Sunday (7)
         */
        uint8_t  weekday;
        /**
         * The number of 1/25 fractions of a second.
         * Valid range 0-255
         */
        uint8_t  fractions256;
        /**
         * Reason(s) for adjusting the time.
         */
        uint8_t  adjust_reason;
    };

    struct EventHandler {
        /**
         * This function is called if the current time characteristic is changed by the client
         */
        virtual void on_current_time_changed(time_t current_time) { }
    };

    /**
     * Initialize the internal BLE object to @p ble and configure the current time characteristic
     * with the appropriate UUID.
     *
     * @param ble BLE object to host the current time service
     *
     * @attention The Initializer must be called after instantiating a current time service.
     */
    CurrentTimeService(BLE &ble);

    ~CurrentTimeService();

    CurrentTimeService(const CurrentTimeService&) = delete;
    CurrentTimeService &operator=(const CurrentTimeService&) = delete;

    /**
     * Set the onCurrentTimeRead() and onCurrentTimeWritten() functions as the read authorization callback
     * and write authorization callback, respectively for the current time characteristic.
     * Add the current time service to the BLE device.
     *
     * @return BLE_ERROR_NONE if the service was successfully added.
     */
    ble_error_t init();

    /**
    * Set the event handler to handle events raised by the current time service.
    *
    * @param handler EventHandler object.
    */
    void set_event_handler(EventHandler *handler);

    /**
     * Get the time in seconds since 00:00 January 1, 1970 plus a configurable offset.
     *
     * @return Time in seconds.
     */
    time_t get_time();

    /**
     * Set the time offset, i.e. the time in seconds beyond Epoch time.
     *
     * @param host_time Time in seconds according to your host.
     *
     */
    void set_time(time_t host_time);

    /**
     * Check all fields of the current time object.
     *
     * @return False, if any of the fields are out of range; otherwise, true.
     */
    bool current_time_is_valid();

private:
    void onDataRead(GattReadAuthCallbackParams *read_request);

    void onDataWritten(GattWriteAuthCallbackParams *write_request);

    void serialize(uint8_t *data, const struct tm *local_time_tm);

    bool deserialize(struct tm *remote_time_tm, const uint8_t *data);

private:
    BLE &_ble;

    CurrentTime _current_time = { 0 };
    ReadWriteGattCharacteristic<CurrentTime> _current_time_char;
    time_t _time_offset = 0;
    EventHandler *_current_time_handler = nullptr;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif // CURRENT_TIME_SERVICE_H
