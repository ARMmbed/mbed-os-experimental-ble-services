/*
 * Copyright (c) 2020, Arm Limited and affiliates.
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

#include "gtest/gtest.h"

/* these are real mbed-os headers but the implementation will be provided by mocks */
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"
#include "ble/GattClient.h"
#include "ble/SecurityManager.h"

#include "ble/gap/ChainableGapEventHandler.h"
#include "ble/gatt/ChainableGattServerEventHandler.h"

#include "ble-service-fota/FOTAService.h"

/* this provides all the ble mocks inside the fake BLE instance */
#include "ble_mocks.h"
/* this is a fake event queue that avoids mbed-os dependencies and allows manual dispatch */
#include "events/EventQueue.h"

#include <chrono>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <string>

using namespace ble;
using namespace events;
using namespace std::chrono_literals;

struct EventHandlerMock : FOTAService::EventHandler {
    MOCK_METHOD(FOTAService::StatusCode_t, on_binary_stream_written, (FOTAService&, mbed::Span<const uint8_t>), (override));
    MOCK_METHOD(GattAuthCallbackReply_t, on_control_written, (FOTAService&, mbed::Span<const uint8_t>), (override));
};

/* This test does not test anything, you may use it as a template for your unit tests.
 * It shows all the elements you need to use mocks for all the ble APIS. */

class TestFOTAService : public testing::Test {
protected:

    BLE* ble;
    events::EventQueue event_queue;
    ChainableGapEventHandler chainable_gap_eh;
    ChainableGattServerEventHandler chainable_gatt_server_eh;

    std::unique_ptr<FOTAService> fota_service;

    void SetUp()
    {
        /* this call uses ble::init_mocks() to initialises the mocks */
        ble = &BLE::Instance();
        GattServer &server = ble->gattServer();
        fota_service = std::make_unique<FOTAService>(*ble, event_queue, chainable_gap_eh, chainable_gatt_server_eh,
                "1.0.0", "1.0.0", "test");
    }

    void TearDown()
    {
        /* remember you must call this at the end of the test if you have any expectations set */
        ble::delete_mocks();
    }
};

struct comp {

    template<typename T>
    bool operator()(const T &lhs, const T &rhs) const {
        return lhs == rhs;
    }

};

TEST_F(TestFOTAService, init)
{
    /* these are the user facing APIs */
    Gap &gap = ble->gap();
    GattServer &server = ble->gattServer();

    // A service with the FOTAService UUID should be added to the gatt server
    EXPECT_CALL(gatt_server_mock(), addService(testing::Property(&GattService::getUUID, uuids::FOTAService::BaseUUID)))
            .Times(1);

    fota_service->init();

    // Capture the link loss service registered in gatt server
    auto service = gatt_server_mock().services[0];

    // The link loss service should contain 5 characteristics
    ASSERT_EQ(service.characteristics.size(), 5);

    std::vector<UUID> uuids = {
            UUID(uuids::FOTAService::BinaryStreamUUID),
            UUID(uuids::FOTAService::ControlUUID),
            UUID(uuids::FOTAService::StatusUUID),
            UUID(uuids::FOTAService::VersionUUID),
            UUID(GattCharacteristic::UUID_FIRMWARE_REVISION_STRING_CHAR)
    };

    /* Make sure each UUID has been added to the service */
    for(auto characteristic : service.characteristics) {
        auto result = std::find_if(uuids.begin(), uuids.end(), [&](UUID uuid) {
            return uuid == characteristic.uuid;
        });

        ASSERT_NE(result, uuids.end());

        if(result != uuids.end()) {
            uuids.erase(result);
        }
    }

    std::map<UUID, unsigned int, comp> props_map {
        { UUID(uuids::FOTAService::BinaryStreamUUID),  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE },
        { UUID(uuids::FOTAService::ControlUUID), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE },
        { UUID(uuids::FOTAService::StatusUUID), (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) },
        { UUID(uuids::FOTAService::VersionUUID), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ },
        { UUID(GattCharacteristic::UUID_FIRMWARE_REVISION_STRING_CHAR), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ },
    };
}
