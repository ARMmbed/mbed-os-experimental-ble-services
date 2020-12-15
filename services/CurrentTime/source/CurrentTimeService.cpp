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

#include "CurrentTimeService.h"

CurrentTimeService::CurrentTimeService(BLE &ble) :
    _ble(ble),
    _current_time_char(GattCharacteristic::UUID_CURRENT_TIME_CHAR, &_current_time)
{
}

CurrentTimeService::~CurrentTimeService()
{
}

ble_error_t CurrentTimeService::init()
{
    GattCharacteristic *charTable[] = {&_current_time_char};
    GattService currentTimeService(GattService::UUID_CURRENT_TIME_SERVICE, charTable, 1);

    _current_time_char.setReadAuthorizationCallback (this, &CurrentTimeService::onDataRead);
    _current_time_char.setWriteAuthorizationCallback(this, &CurrentTimeService::onDataWritten);

    ble_error_t bleError = _ble.gattServer().addService(currentTimeService);

    MBED_ASSERT(sizeof(_current_time) == CURRENT_TIME_CHAR_VALUE_SIZE);

    return bleError;
}

void CurrentTimeService::set_event_handler(EventHandler *handler) {
    _current_time_handler = handler;
}

time_t CurrentTimeService::get_time()
{
    time_t epoch_time = time(nullptr);

    return epoch_time + _time_offset;
}

void CurrentTimeService::set_time(time_t new_time)
{
    time_t epoch_time = time(nullptr);

    _time_offset = new_time - epoch_time;
}

bool CurrentTimeService::current_time_is_valid()
{
    bool result = true;

    if ((_current_time.year    < 1582) || (_current_time.year    > 9999)) {
        result = false;
    }
    if ((_current_time.month   <    1) || (_current_time.month   >   12)) {
        result = false;
    }
    if ((_current_time.day     <    1) || (_current_time.day     >   31)) {
        result = false;
    }
    if ((_current_time.hours   <    0) || (_current_time.hours   >   23)) {
        result = false;
    }
    if ((_current_time.minutes <    0) || (_current_time.minutes >   59)) {
        result = false;
    }
    if ((_current_time.seconds <    0) || (_current_time.seconds >   59)) {
        result = false;
    }
    if ((_current_time.weekday <    1) || (_current_time.weekday >    7)) {
        result = false;
    }

    return result;
}

void CurrentTimeService::onDataRead(GattReadAuthCallbackParams *read_request)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(&_current_time);

    time_t local_time = get_time();

    struct tm *local_time_tm = localtime(&local_time);

    serialize(data, local_time_tm);

    read_request->data = reinterpret_cast<uint8_t *>(&_current_time);
    read_request->len  = CURRENT_TIME_CHAR_VALUE_SIZE;
    read_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

void CurrentTimeService::onDataWritten(GattWriteAuthCallbackParams *write_request)
{
    const uint8_t *data = write_request->data;

    struct tm remote_time_tm{};

    write_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;

    if (!deserialize(&remote_time_tm, data)) {
        /* The alert level is out of range
         * */
        write_request->authorizationReply = static_cast<GattAuthCallbackReply_t>(0xFF);
    }

    time_t remote_time = mktime(&remote_time_tm);

    set_time(remote_time);

    if (_current_time_handler) {
        _current_time_handler->on_current_time_written(remote_time);
    }
}

void CurrentTimeService::serialize(uint8_t *data, const struct tm *local_time_tm)
{
    *data++ = (local_time_tm->tm_year + 1900);
    *data++ = (local_time_tm->tm_year + 1900) >> 8;
    *data++ =  local_time_tm->tm_mon  + 1;
    *data++ =  local_time_tm->tm_mday;
    *data++ =  local_time_tm->tm_hour;
    *data++ =  local_time_tm->tm_min;
    *data++ =  local_time_tm->tm_sec;
    /*
     * The tm_wday field of a tm struct means days since Sunday (0-6)
     * However, the weekday field of a CurrentTime struct means Mon-Sun (1-7)
     * So, if tm_wday = 0, i.e. Sunday, the correct value for weekday is 7
     * Otherwise, the fields signify the same days and no correction is needed
     * */
    *data++ =  local_time_tm->tm_wday == 0 ? 7 : local_time_tm ->tm_wday;
    *data   =  0;
}

bool CurrentTimeService::deserialize(struct tm *remote_time_tm, const uint8_t *data)
{
    bool deserialization_is_possible;

    _current_time.year    = *data | (*(data + 1) << 8);
    _current_time.month   = *data++;
    _current_time.day     = *data++;
    _current_time.hours   = *data++;
    _current_time.minutes = *data++;
    _current_time.seconds = *data++;
    _current_time.weekday = *data++;

    if (current_time_is_valid()) {
        remote_time_tm->tm_year  =  _current_time.year - 1900;
        data += 2;
        remote_time_tm->tm_mon   =  _current_time.month - 1;
        remote_time_tm->tm_mday  =  _current_time.day;
        remote_time_tm->tm_hour  =  _current_time.hours;
        remote_time_tm->tm_min   =  _current_time.minutes;
        remote_time_tm->tm_sec   =  _current_time.seconds;
        /*
         * The weekday field of a CurrentTime struct means Mon-Sun (1-7)
         * However, the tm_wday field of a tm_day struct means days since Sunday (0-6)
         * So, if weekday = 7, i.e. Sunday, the correct value for tm_wday is 0
         * Otherwise, the fields signify the same days and no correction is needed
         * */
        remote_time_tm->tm_wday  = (_current_time.weekday == 7 ? 0 : _current_time.weekday);

        deserialization_is_possible = true;
    } else {
        /* Invalid data written to CurrentTime struct, can't deserialize
         * */
        _current_time = { 0 };
        deserialization_is_possible = false;
    }

    return deserialization_is_possible;
}
