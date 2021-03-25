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

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble-service-fota/FOTAService.h"

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "FOTA";

static events::EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);
static ChainableGapEventHandler chainable_gap_event_handler;
static ChainableGattServerEventHandler chainable_gatt_server_event_handler;

class TestFOTAService : public FOTAService {

public:

    TestFOTAService(BLE &ble, events::EventQueue &event_queue,
            ChainableGapEventHandler &chainable_gap_eh,
            ChainableGattServerEventHandler &chainable_gatt_server_eh,
            const char *protocol_version, const char *fw_rev = nullptr,
            const char *dev_desc = nullptr) : FOTAService(ble, event_queue,
                    chainable_gap_eh, chainable_gatt_server_eh, protocol_version,
                    fw_rev, dev_desc) {
    }

    /* Allow the host test to remotely set the expected fragment ID */
    void set_fragment_id(uint8_t id) {
        _fragment_id = id;
    }

};

class FOTAServiceDemo : ble::Gap::EventHandler, FOTAService::EventHandler {

public:

    enum CustomOpCodes_t {

        OP_CODE_SET_XOFF        = 0x41,
        OP_CODE_SET_XON         = 0x42,
        OP_CODE_SET_FRAGMENT_ID = 0x43

    };

public:
    FOTAServiceDemo(BLE &ble, events::EventQueue &event_queue, ChainableGapEventHandler &chainable_gap_eh,
            ChainableGattServerEventHandler &chainable_gatt_server_eh) :
            _ble(ble),
            _event_queue(event_queue),
            _chainable_gap_eh(chainable_gap_eh),
            _chainable_gatt_server_eh(chainable_gatt_server_eh),
            _fota_service(_ble, _event_queue, _chainable_gap_eh, _chainable_gatt_server_eh,
                    "1.0.0", "1.0.0", "test"),
            _adv_data_builder(_adv_buffer)
    {
    }

    void start()
    {
        _ble.init(this, &FOTAServiceDemo::on_init_complete);

        _event_queue.dispatch_forever();
    }

private:
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params)
    {
        if (params->error != BLE_ERROR_NONE) {
            printf("Ble initialization failed.");
            return;
        }

        /* The ChainableGapEventHandler allows us to dispatch events from GAP to more than a single event handler */
        _chainable_gap_eh.addEventHandler(this);
        _ble.gap().setEventHandler(&_chainable_gap_eh);
        _ble.gattServer().setEventHandler(&_chainable_gatt_server_eh);

        _fota_service.init();

        _fota_service.set_event_handler(this);

        start_advertising();
    }

    void start_advertising()
    {
        ble::AdvertisingParameters adv_parameters(
                ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
                ble::adv_interval_t(ble::millisecond_t(100))
        );

        _adv_data_builder.setFlags();
        _adv_data_builder.setAppearance(ble::adv_data_appearance_t::UNKNOWN);
        _adv_data_builder.setName(DEVICE_NAME);

        ble_error_t error = _ble.gap().setAdvertisingParameters(
                ble::LEGACY_ADVERTISING_HANDLE,
                adv_parameters
        );

        if (error) {
            printf("_ble.gap().setAdvertisingParameters() failed\r\n");
            return;
        }

        error = _ble.gap().setAdvertisingPayload(
                ble::LEGACY_ADVERTISING_HANDLE,
                _adv_data_builder.getAdvertisingData()
        );

        if (error) {
            printf("_ble.gap().setAdvertisingPayload() failed\r\n");
            return;
        }

        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            printf("_ble.gap().startAdvertising() failed\r\n");
            return;
        }

        printf("Device advertising, please connect\r\n");
    }

private:
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override
    {
        if (event.getStatus() == ble_error_t::BLE_ERROR_NONE) {
            printf("Client connected, you may now subscribe to updates\r\n");
        }
    }

    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override
    {
        printf("Client disconnected, restarting advertising\r\n");

        ble_error_t error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            printf("_ble.gap().startAdvertising() failed\r\n");
            return;
        }
    }

    FOTAService::StatusCode_t on_binary_stream_written(FOTAService &svc, mbed::Span<const uint8_t> buffer) override {
       /* Write data to the UART for the host test to check */
        printf("bsc written: ");
        for(int i = 0; i < buffer.size(); i++) {
            printf("%X", buffer[i]);
        }
        printf("\r\n");
        return FOTAService::FOTA_STATUS_OK;
    }

    GattAuthCallbackReply_t on_control_written(FOTAService &svc, mbed::Span<const uint8_t> buffer) override {
        switch(buffer[0]) {
        case FOTAService::FOTA_NO_OP:
        {
            break;
        }

        case FOTAService::FOTA_START:
        {
            svc.start_fota_session();
            uint8_t status_code = FOTAService::FOTA_STATUS_OK;
            svc.notify_status(mbed::make_const_Span(&status_code, 1));
            printf("fota started\r\n");
            break;
        }

        case FOTAService::FOTA_STOP:
        {
            svc.stop_fota_session();
            break;
        }

        case FOTAService::FOTA_COMMIT:
        {
            printf("fota commit\r\n");
            svc.stop_fota_session();
            break;
        }

        /* Custom commands for test */
        case OP_CODE_SET_XOFF:
        {
            printf("setting xoff\r\n");
            svc.set_xoff();
            break;
        }

        case OP_CODE_SET_XON:
        {
            printf("setting xon\r\n");
            svc.set_xon();
            break;
        }

        case OP_CODE_SET_FRAGMENT_ID:
        {
            printf("setting fragment id: %d", buffer[1]);
            // TODO is having the svc parameter provided to this handler necessary?
            _fota_service.set_fragment_id(buffer[1]);
            break;
        }

        default:
        {
            return (GattAuthCallbackReply_t) FOTAService::AUTH_CALLBACK_REPLY_ATTERR_UNSUPPORTED_OPCODE;
            break;
        }
        }

        return AUTH_CALLBACK_REPLY_SUCCESS;

    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;
    ChainableGapEventHandler &_chainable_gap_eh;
    ChainableGattServerEventHandler &_chainable_gatt_server_eh;

    TestFOTAService _fota_service;

    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
};

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    event_queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    FOTAServiceDemo demo(ble, event_queue, chainable_gap_event_handler,
            chainable_gatt_server_event_handler);
    demo.start();

    return 0;
}
