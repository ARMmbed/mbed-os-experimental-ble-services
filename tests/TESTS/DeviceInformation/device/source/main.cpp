/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
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

#include "ble/BLE.h"
#include "ble-service-device-information/DeviceInformationService.h"
#include "events/EventQueue.h"

const static char DEVICE_NAME[] = "DeviceInformation";

class DeviceInformationTest {
public:
    DeviceInformationTest(BLE &ble) : _ble(ble)
    {
        _ble.init(this, &DeviceInformationTest::on_init_complete);
    }

private:
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params)
    {
        DeviceInformationService::system_id_t system_id;
        system_id.manufacturer_defined_identifier = 1;
        system_id.organizationally_unique_identifier = 2;

        DeviceInformationService::regulatory_cert_data_list_t cert_data_list;
        uint8_t data[2] { /* size of data */1, /* data */2 };
        cert_data_list.data = data;

        DeviceInformationService::pnp_id_t pnp_id;
        pnp_id.vendor_id_source = 1;
        pnp_id.vendor_id = 2;
        pnp_id.product_id = 3;
        pnp_id.product_version = 4;

        DeviceInformationService::add_service(
            _ble,
            "manufacturers_name",
            "model_number",
            "serial_number",
            "hardware_revision",
            "firmware_revision",
            "software_revision",
            &system_id,
            &cert_data_list,
            &pnp_id
        );

        start_advertising();
    }

    void start_advertising()
    {
        uint8_t adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
        ble::AdvertisingDataBuilder adv_data_builder(adv_buffer);
        adv_data_builder.setFlags();
        adv_data_builder.setName(DEVICE_NAME);

        _ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, ble::AdvertisingParameters());
        _ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE, adv_data_builder.getAdvertisingData());

        ble_error_t error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            printf("ERROR startAdvertising() failed (%d)\r\n", error);
            return;
        }

        printf("ready\r\n");
    }

private:
    BLE &_ble;
};

static events::EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);
    DeviceInformationTest test(ble);
    event_queue.dispatch_forever();
    return 0;
}