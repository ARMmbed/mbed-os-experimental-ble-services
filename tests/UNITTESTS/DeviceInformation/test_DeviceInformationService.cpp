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

#include "ble/BLE.h"
#include "ble/GattServer.h"

#include "ble-service-device-information/DeviceInformationService.h"

#include "ble_mocks.h"

using namespace ble;

using ::testing::Property;

class TestDeviceInformationService : public testing::Test {
protected:
    BLE *ble;

    void SetUp()
    {
        ble = &BLE::Instance();
    }

    void TearDown()
    {
        ble::delete_mocks();
    }
};


TEST_F(TestDeviceInformationService, add_empty)
{
    GattServer &server = ble->gattServer();

    EXPECT_CALL(gatt_server_mock(), addService(Property(&GattService::getUUID, GattService::UUID_DEVICE_INFORMATION_SERVICE)))
            .Times(1);

    DeviceInformationService::add_service(*ble);

    GattServerMock::service_t& service = gatt_server_mock().services[0];

    /* service should have no characteristics */
    ASSERT_EQ(service.characteristics.size(), 0);
}

TEST_F(TestDeviceInformationService, add_all)
{
    GattServer &server = ble->gattServer();

    EXPECT_CALL(gatt_server_mock(), addService(Property(&GattService::getUUID, GattService::UUID_DEVICE_INFORMATION_SERVICE)))
            .Times(1);

    DeviceInformationService::system_id_t system_id;

    DeviceInformationService::regulatory_cert_data_list_t cert_data_list;
    uint8_t data[4];
    /* the data definition mandates size as first byte */
    data[0] = 3;
    cert_data_list.data = (uint8_t*)data;

    DeviceInformationService::pnp_id_t pnp_id;

    DeviceInformationService::add_service(
        *ble,
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

    GattServerMock::service_t& service = gatt_server_mock().services[0];

    ASSERT_EQ(service.characteristics.size(), 9);

    UUID uuids[9] = {
        GattCharacteristic::UUID_MANUFACTURER_NAME_STRING_CHAR,
        GattCharacteristic::UUID_MODEL_NUMBER_STRING_CHAR,
        GattCharacteristic::UUID_SERIAL_NUMBER_STRING_CHAR,
        GattCharacteristic::UUID_HARDWARE_REVISION_STRING_CHAR,
        GattCharacteristic::UUID_FIRMWARE_REVISION_STRING_CHAR,
        GattCharacteristic::UUID_SOFTWARE_REVISION_STRING_CHAR,
        GattCharacteristic::UUID_SYSTEM_ID_CHAR,
        GattCharacteristic::UUID_IEEE_REGULATORY_CERTIFICATION_DATA_LIST_CHAR,
        GattCharacteristic::UUID_PNP_ID_CHAR
    };

    /* each uuid must be present once */
    for (size_t i = 0; i < 9; i++) {
        size_t found = 0;
        for (size_t j = 0; j < 9; j++) {
            if (service.characteristics[j].uuid == uuids[i]) {
                found++;
            }
        }
        ASSERT_EQ(found, 1);
    }

    /* all should be readable but not writable */
    for (size_t i = 0; i < 9; i++) {
        ASSERT_TRUE(service.characteristics[i].properties & (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ));
        ASSERT_FALSE(service.characteristics[i].properties & (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE));
    }
}


TEST_F(TestDeviceInformationService, add_with_gaps)
{
    GattServer &server = ble->gattServer();

    EXPECT_CALL(gatt_server_mock(), addService(Property(&GattService::getUUID, GattService::UUID_DEVICE_INFORMATION_SERVICE)))
          .Times(1);

    DeviceInformationService::system_id_t system_id;

    DeviceInformationService::add_service(
        *ble,
        "manufacturers_name",
        nullptr,
        "serial_number",
        nullptr,
        nullptr,
        nullptr,
        &system_id
    );

    GattServerMock::service_t& service = gatt_server_mock().services[0];

    ASSERT_EQ(service.characteristics.size(), 3);

    UUID uuids[3] = {
        GattCharacteristic::UUID_MANUFACTURER_NAME_STRING_CHAR,
        GattCharacteristic::UUID_SERIAL_NUMBER_STRING_CHAR,
        GattCharacteristic::UUID_SYSTEM_ID_CHAR,
    };

    /* each uuid must be present once */
    for (size_t i = 0; i < 3; i++) {
        size_t found = 0;
        for (size_t j = 0; j < 3; j++) {
            if (service.characteristics[j].uuid == uuids[i]) {
                found++;
            }
        }
        ASSERT_EQ(found, 1);
    }
}
