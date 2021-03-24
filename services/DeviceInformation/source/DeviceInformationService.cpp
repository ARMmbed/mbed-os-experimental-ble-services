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

#include "ble-service-device-information/DeviceInformationService.h"

#if BLE_FEATURE_GATT_SERVER

ble_error_t DeviceInformationService::add_service(
    BLE &ble,
    const char *manufacturers_name,
    const char *model_number,
    const char *serial_number,
    const char *hardware_revision,
    const char *firmware_revision,
    const char *software_revision,
    const system_id_t *system_id,
    const regulatory_cert_data_list_t *cert_data_list,
    const pnp_id_t *pnp_id
) {
    /* all characteristics are optional so we need to build an array that contains the requested ones */
    size_t param_index = 0;

    const char* param_strings[6];
    UUID param_string_uuids[6];

    if (manufacturers_name) {
        param_strings[param_index] = manufacturers_name;
        param_string_uuids[param_index++] = GattCharacteristic::UUID_MANUFACTURER_NAME_STRING_CHAR;
    }
    if (model_number) {
        param_strings[param_index] = model_number;
        param_string_uuids[param_index++] = GattCharacteristic::UUID_MODEL_NUMBER_STRING_CHAR;
    }
    if (serial_number) {
        param_strings[param_index] = serial_number;
        param_string_uuids[param_index++] = GattCharacteristic::UUID_SERIAL_NUMBER_STRING_CHAR;
    }
    if (hardware_revision) {
        param_strings[param_index] = hardware_revision;
        param_string_uuids[param_index++] = GattCharacteristic::UUID_HARDWARE_REVISION_STRING_CHAR;
    }
    if (firmware_revision) {
        param_strings[param_index] = firmware_revision;
        param_string_uuids[param_index++] = GattCharacteristic::UUID_FIRMWARE_REVISION_STRING_CHAR;
    }
    if (software_revision) {
        param_strings[param_index] = software_revision;
        param_string_uuids[param_index++] = GattCharacteristic::UUID_SOFTWARE_REVISION_STRING_CHAR;
    }

    GattCharacteristic* param_chars[9];

    for (int i = 0; i < param_index; i++) {
        param_chars[i] = new GattCharacteristic(
            param_string_uuids[i],
            (uint8_t *)param_strings[i],
            strlen(param_strings[i]), /* Min length */
            strlen(param_strings[i]), /* Max length */
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ
        );
    }

    uint8_t system_id_value[8];

    if (system_id) {
        /* pack the values */
        for (int i = 0; i < 5; i++) {
            system_id_value[i] = system_id->manufacturer_defined_identifier >> (i * 8);
        }
        for (int i = 0; i < 3; i++) {
            system_id_value[5 + i] = system_id->organizationally_unique_identifier >> (i * 8);
        }

        param_chars[param_index++] = new GattCharacteristic(
            GattCharacteristic::UUID_SYSTEM_ID_CHAR,
            system_id_value,
            8, /* Min length */
            8, /* Max length */
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ
        );
    }

    if (cert_data_list && cert_data_list->data) {
        param_chars[param_index++] = new GattCharacteristic(
            GattCharacteristic::UUID_IEEE_REGULATORY_CERTIFICATION_DATA_LIST_CHAR,
            (uint8_t*)cert_data_list->data,
            cert_data_list->data[0] + 1, /* Min length */
            cert_data_list->data[0] + 1, /* Max length */
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ
        );
    }

    uint8_t pnp_value[7];

    if (pnp_id) {
        /* pack the values */
        pnp_value[0] = pnp_id->vendor_id_source;
        pnp_value[1] = pnp_id->vendor_id;
        pnp_value[2] = pnp_id->vendor_id >> 8;
        pnp_value[3] = pnp_id->product_id;
        pnp_value[4] = pnp_id->product_id >> 8;
        pnp_value[5] = pnp_id->product_version;
        pnp_value[6] = pnp_id->product_version >> 8;

        param_chars[param_index++] = new GattCharacteristic(
            GattCharacteristic::UUID_PNP_ID_CHAR,
            pnp_value,
            7, /* Min length */
            7, /* Max length */
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ
        );
    }

    GattService deviceInformationService(GattService::UUID_DEVICE_INFORMATION_SERVICE, param_chars, param_index);

    ble_error_t status = ble.gattServer().addService(deviceInformationService);

    for (int i = 0; i < param_index; i++) {
        delete param_chars[i];
    }

    return status;
}

#endif // BLE_FEATURE_GATT_SERVER
