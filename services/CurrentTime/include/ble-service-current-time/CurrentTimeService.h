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

#include <time.h>

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
    struct CurrentTime {
        uint16_t year;
        uint8_t  month;
        uint8_t  day;
        uint8_t  hours;
        uint8_t  minutes;
        uint8_t  seconds;

        uint8_t  weekday;

        uint8_t  fractions256;

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

    }

    /**
     * Construct and initialize a current time service.
     *
     * @param ble
     * @param event_queue
     * @param chainable_gap_event_handler
     */
    CurrentTimeService(BLE &ble, events::EventQueue &event_queue) :
            _ble(ble),
            _event_queue(event_queue),
            _current_time_char(GattCharacteristic::UUID_CURRENT_TIME_CHAR, &_current_time) {
        memset(&_current_time, 0, sizeof(_current_time));
    }

    ble_error_t init() {
        GattCharacteristic *charTable[] = {&_current_time_char};
        GattService currentTimeService(GattService::UUID_CURRENT_TIME_SERVICE, charTable, 1);

        _current_time_char.setReadAuthorizationCallback(this, &CurrentTimeService::onDataRead);
        _current_time_char.setReadAuthorizationCallback(this, &CurrentTimeService::onDataRead);

        ble_error_t bleError = _ble.gattServer().addService(currentTimeService);

        return bleError;
    }

    time_t get_current_time() {
        time_t t = time(NULL);

        return _current_time_offset + t;
    }

    void set_time(time_t new_time) {
        time_t t = time(NULL);

        _current_time_offset = t - new_time;
    }

private:
    void set_current_time() {
        timeval tv{};
        gettimeofday(&tv, NULL);
        // printf("%u\r\n", (unsigned int) tv.tv_sec);

        struct tm *current_time;
        time_t local_time = tv.tv_sec + _current_time_offset;
        current_time = localtime(&local_time);

        _current_time.year    = current_time->tm_year + 1900;
        _current_time.month   = current_time->tm_mon  + 1;
        _current_time.day     = current_time->tm_mday;
        _current_time.hours   = current_time->tm_hour;
        _current_time.minutes = current_time->tm_min;
        _current_time.seconds = current_time->tm_sec;
        _current_time.weekday = (current_time->tm_wday == 0) ? 7 : current_time->tm_wday;
        // 1/256th of seconds not supported
        _current_time.fractions256 = 0;
    }

    void onDataRead(GattReadAuthCallbackParams *read_request) {
        set_current_time();

        _ble.gattServer().write(_current_time_char.getValueHandle(),
                                reinterpret_cast<const uint8_t *>(&_current_time),
                                sizeof(_current_time));

        read_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    }

    void onDataWritten(GattWriteAuthCallbackParams *write_request) {
        const uint8_t *ct = (write_request->data);

        struct tm *current_time;

        current_time->tm_year = (*ct | (*(ct + 1) << 8)) - 1900;
        ct += 2;
        current_time->tm_mon  =  *ct++ - 1;
        current_time->tm_mday =  *ct++;
        current_time->tm_hour =  *ct++;
        current_time->tm_min  =  *ct++;
        current_time->tm_sec  =  *ct++;
        current_time->tm_wday =  (*ct == 7 ? 0 : *ct);

        time_t remote_time = mktime(current_time);

        set_time(remote_time);
    }

    BLE &_ble;
    events::EventQueue &_event_queue;

    CurrentTime _current_time;
    ReadWriteGattCharacteristic<CurrentTime> _current_time_char;
    time_t _current_time_offset = 0;
    EventHandler *_alert_handler = nullptr;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif // BLE_CURRENT_TIME_SERVICE_H
