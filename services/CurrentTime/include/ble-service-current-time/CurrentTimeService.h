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

#ifndef BLE_CURRENT_TIME_SERVICE_H
#define BLE_CURRENT_TIME_SERVICE_H

#ifdef BLE_FEATURE_GATT_SERVER

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "events/EventQueue.h"
#include "mbed_rtc_time.h"

#include <ctime>

#define CURRENT_TIME_CHAR_VALUE_SIZE 10

/**
 * BLE Current Time Service
 *
 * @par purpose
 *
 * @par usage
 *
 * @note The specification for the current time service can be found here:
 * https://www.bluetooth.com/specifications/gatt
 *
 * @attention The user should not instantiate more than a single current time service service
 */
class CurrentTimeService {
public:
    MBED_PACKED(struct) CurrentTime {
        /* Year as defined by the Gregorian calendar.
         * Valid range 1582 to 9999.
         * */
        uint16_t year;
        /* Month of the year as defined by the Gregorian calendar.
         * Valid range 1 (January) to 12 (December).
         * */
        uint8_t  month;
        /* Day of the month as defined by the Gregorian calendar.
         * Valid range 1 to 31.
         * */
        uint8_t  day;
        /* Number of hours past midnight.
         * Valid range 0 to 23.
         * */
        uint8_t  hours;
        /* Number of minutes since the start of the hour.
         * Valid range 0 to 59.
         * */
        uint8_t  minutes;
        /* Number of seconds since the start of the minute.
         * Valid range 0 to 59.
         * */
        uint8_t  seconds;
        /* ... */
        uint8_t  weekday;
        /* ... */
        uint8_t  fractions256;
        /* ... */
        uint8_t  adjustReason;
    };

    struct EventHandler {
        /**
         * On current time changed
         *
         * This function is called if the current time characteristic is changed by the client
         */
        virtual void on_current_time_written(time_t current_time) { }
    };

    /**
     * Set event handler
     *
     * @param handler EventHandler object to handle events raised by the current time service
     */
    void set_event_handler(EventHandler *handler) {
        _current_time_handler = handler;
    }

    /**
     * Constructor
     *
     * @param ble
     */
    CurrentTimeService(BLE &ble);

    /**
     * Destructor
     */
    ~CurrentTimeService();

    CurrentTimeService(const CurrentTimeService&) = delete;
    CurrentTimeService &operator=(const CurrentTimeService&) = delete;

    /**
     * Initializer
     *
     * @return
     */
    ble_error_t init();

    /**
     * Get time
     *
     * @return
     */
    time_t get_time();

    /**
     * Set time
     *
     * @param new_time
     */
    void set_time(time_t new_time);

//    /**
//     * Validate all fields of CurrentTime struct
//     *
//     * @return
//     */
//    bool validate() {
//
//    }

private:
    void onDataRead(GattReadAuthCallbackParams *read_request);

    void onDataWritten(GattWriteAuthCallbackParams *write_request);

    void serialize(uint8_t *data, const struct tm *local_time_tm);

    void deserialize(struct tm *remote_time_tm, const uint8_t *data);

private:
    BLE &_ble;

    CurrentTime _current_time = { 0 };
    ReadWriteGattCharacteristic<CurrentTime> _current_time_char;
    time_t _time_offset = 0;
    EventHandler *_current_time_handler = nullptr;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif // BLE_CURRENT_TIME_SERVICE_H
