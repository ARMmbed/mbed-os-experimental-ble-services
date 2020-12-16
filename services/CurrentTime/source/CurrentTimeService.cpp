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

#define CURRENT_TIME_CHAR_VALUE_SIZE 10
#define DATA_FIELD_IGNORED 0x80

CurrentTimeService::CurrentTimeService(BLE &ble) :
    _ble(ble),
    _current_time_char(GattCharacteristic::UUID_CURRENT_TIME_CHAR, &_current_time)
{
}

ble_error_t CurrentTimeService::init()
{
    GattCharacteristic *charTable[] = {&_current_time_char};
    GattService currentTimeService(GattService::UUID_CURRENT_TIME_SERVICE, charTable, 1);

    _current_time_char.setReadAuthorizationCallback (this, &CurrentTimeService::onCurrentTimeRead);
    _current_time_char.setWriteAuthorizationCallback(this, &CurrentTimeService::onCurrentTimeWritten);

    ble_error_t bleError = _ble.gattServer().addService(currentTimeService);

    MBED_STATIC_ASSERT(sizeof(_current_time) == CURRENT_TIME_CHAR_VALUE_SIZE, "Current time characteristic value size = 10");

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

void CurrentTimeService::set_time(time_t host_time)
{
    time_t epoch_time = time(nullptr);

    _time_offset = host_time - epoch_time;
}

void CurrentTimeService::onCurrentTimeRead(GattReadAuthCallbackParams *read_request)
{
    time_t local_time = get_time();
    CurrentTime local_current_time(localtime(&local_time));

    if (local_current_time.valid()) {
        // copy into local buffer for current time
        _current_time = local_current_time;
        read_request->data = reinterpret_cast<uint8_t *>(&_current_time);
        read_request->len  = CURRENT_TIME_CHAR_VALUE_SIZE;
        read_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
    } else {
        read_request->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_UNLIKELY_ERROR;
    }
}

void CurrentTimeService::onCurrentTimeWritten(GattWriteAuthCallbackParams *write_request)
{
    CurrentTime input_time(write_request->data);

    // Validate data in input
    if (!input_time.valid()) {
        write_request->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_OUT_OF_RANGE;
        return;
    }

    // report error for fields not handled
    if (input_time.fractions256 || input_time.adjust_reason) {
        write_request->authorizationReply = (GattAuthCallbackReply_t) DATA_FIELD_IGNORED;
        return;
    }

    // Process time in input, set it and report the change in the event handler
    struct tm remote_time_tm{};
    input_time.to_tm(&remote_time_tm);
    time_t remote_time = mktime(&remote_time_tm);

    set_time(remote_time);

    if (_current_time_handler) {
        _current_time_handler->on_current_time_changed(remote_time);
    }

    write_request->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

CurrentTimeService::CurrentTime::CurrentTime(const uint8_t *data)
{
    // FIXME: 2bit transmitted in little endian
    year    = *data | (*(data + 1) << 8);
    month   = *data++;
    day     = *data++;
    hours   = *data++;
    minutes = *data++;
    seconds = *data++;
    weekday = *data++;
    fractions256 = *data++;
    adjust_reason = *data;
}

CurrentTimeService::CurrentTime::CurrentTime(const struct tm *local_time_tm)
{
    // FIXME: tm_year should be coded in little endian
    year = (local_time_tm->tm_year + 1900);
    month = local_time_tm->tm_mon  + 1;
    day =  local_time_tm->tm_mday;
    hours =  local_time_tm->tm_hour;
    minutes =  local_time_tm->tm_min;
    seconds =  local_time_tm->tm_sec;
    /*
     * The tm_wday field of a tm struct means days since Sunday (0-6)
     * However, the weekday field of a CurrentTime struct means Mon-Sun (1-7)
     * So, if tm_wday = 0, i.e. Sunday, the correct value for weekday is 7
     * Otherwise, the fields signify the same days and no correction is needed
     * */
    weekday = local_time_tm->tm_wday == 0 ? 7 : local_time_tm ->tm_wday;
    fractions256 = 0;
    adjust_reason = 0;
}


bool CurrentTimeService::CurrentTime::valid()
{
    if ((year    < 1582) || (year    > 9999)) {
        return false;
    }
    if ((month   <    1) || (month   >   12)) {
        return false;
    }
    if ((day     <    1) || (day     >   31)) {
        return false;
    }
    if ( hours   >   23) {
        return false;
    }
    if ( minutes >   59) {
        return false;
    }
    if ( seconds >   59) {
        return false;
    }
    if ((weekday <    1) || (weekday >    7)) {
        return false;
    }

    return true;
}

bool CurrentTimeService::CurrentTime::to_tm(struct tm * remote_time_tm)
{
    if (!valid()) {
        return false;
    }

    remote_time_tm->tm_year  =  year - 1900;
    remote_time_tm->tm_mon   =  month - 1;
    remote_time_tm->tm_mday  =  day;
    remote_time_tm->tm_hour  =  hours;
    remote_time_tm->tm_min   =  minutes;
    remote_time_tm->tm_sec   =  seconds;

    /*
     * The weekday field of a CurrentTime struct means Mon-Sun (1-7)
     * However, the tm_wday field of a tm_day struct means days since Sunday (0-6)
     * So, if weekday = 7, i.e. Sunday, the correct value for tm_wday is 0
     * Otherwise, the fields signify the same days and no correction is needed
     * */
    remote_time_tm->tm_wday  = (weekday == 7 ? 0 : weekday);

    return true;
}

