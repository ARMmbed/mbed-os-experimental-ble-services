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
class CurrentTimeService : private ble::Gap::EventHandler {
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
         *
         * @attention This is an abstract function and should be overridden by the user.
         */
        virtual void on_current_time_written(time_t current_time) {}
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
     * Construct and initialize a current time service.
     *
     * @param ble
     */
    CurrentTimeService(BLE &ble) :
            _ble(ble),
            _current_time_char(GattCharacteristic::UUID_CURRENT_TIME_CHAR, &_current_time)
    {
    }

    /**
     *
     * @return
     */
    ble_error_t init() {
        GattCharacteristic *charTable[] = {&_current_time_char};
        GattService currentTimeService(GattService::UUID_CURRENT_TIME_SERVICE, charTable, 1);

        _current_time_char.setReadAuthorizationCallback (this, &CurrentTimeService::onDataRead);
        _current_time_char.setWriteAuthorizationCallback(this, &CurrentTimeService::onDataWritten);

        ble_error_t bleError = _ble.gattServer().addService(currentTimeService);

        MBED_ASSERT(sizeof(_current_time) == CURRENT_TIME_CHAR_VALUE_SIZE);

        return bleError;
    }

    /**
     *
     * @return
     */
    time_t get_time() {
        time_t epoch_time = time(nullptr);

        return epoch_time + _time_offset;
    }

    /**
     *
     * @param new_time
     */
    void set_time(time_t new_time) {
        time_t epoch_time = time(nullptr);

        _time_offset = new_time - epoch_time;
    }

private:
    void onDataRead(GattReadAuthCallbackParams *read_request) {
        uint8_t *data_ptr = reinterpret_cast<uint8_t *>(&_current_time);

        timeval epoch_time{};
        gettimeofday(&epoch_time, nullptr);
        time_t local_time = epoch_time.tv_sec + _time_offset;

        struct tm *current_time = localtime(&local_time);
        
        *data_ptr++ = (current_time->tm_year + 1900);
        *data_ptr++ = (current_time->tm_year + 1900) >> 8;
        *data_ptr++ =  current_time->tm_mon  + 1;
        *data_ptr++ =  current_time->tm_mday;
        *data_ptr++ =  current_time->tm_hour;
        *data_ptr++ =  current_time->tm_min;
        *data_ptr++ =  current_time->tm_sec;
        /*
         * The tm_wday field of a tm struct means days since Sunday (0-6)
         * However, the weekday field of a CurrentTime struct means Mon-Sun (1-7)
         * So, if tm_wday = 0, i.e. Sunday, the correct value for weekday is 7
         * Otherwise, the fields signify the same days and no correction is needed
        */
        *data_ptr++ = (current_time->tm_wday == 0) ? 7 : current_time->tm_wday;
        *data_ptr   =  0;

        read_request->data = reinterpret_cast<uint8_t *>(&_current_time);
        read_request->len  = CURRENT_TIME_CHAR_VALUE_SIZE;
        read_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }

    void onDataWritten(GattWriteAuthCallbackParams *write_request) {
        const uint8_t *data_ptr = write_request->data;

        struct tm current_time{};

        current_time.tm_year  = (*data_ptr | (*(data_ptr + 1) << 8)) - 1900;
        data_ptr += 2;
        current_time.tm_mon   =  *data_ptr++ - 1;
        current_time.tm_mday  =  *data_ptr++;
        current_time.tm_hour  =  *data_ptr++;
        current_time.tm_min   =  *data_ptr++;
        current_time.tm_sec   =  *data_ptr++;
        /*
         * The weekday field of a CurrentTime struct means Mon-Sun (1-7)
         * However, the tm_wday field of a tm_day struct means days since Sunday (0-6)
         * So, if weekday = 7, i.e. Sunday, the correct value for tm_wday is 0
         * Otherwise, the fields signify the same days and no correction is needed
        */
        current_time.tm_wday  = (*data_ptr == 7 ? 0 : *data_ptr);
        current_time.tm_isdst =  0;

        time_t remote_time = mktime(&current_time);

        set_time(remote_time);

        if (_current_time_handler) {
            _current_time_handler->on_current_time_written(remote_time);
        }

        write_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }

private:
    BLE &_ble;

    CurrentTime _current_time = { 0 };
    ReadWriteGattCharacteristic<CurrentTime> _current_time_char;
    time_t _time_offset = 0;
    EventHandler *_current_time_handler = nullptr;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif // BLE_CURRENT_TIME_SERVICE_H
