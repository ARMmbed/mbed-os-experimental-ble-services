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

#ifndef BLE_DEVICE_INFORMATION_SERVICE_H
#define BLE_DEVICE_INFORMATION_SERVICE_H

#include "ble/BLE.h"

#if BLE_FEATURE_GATT_SERVER

/** The Device Information Service exposes manufacturer and/or vendor information about a device.
 *
 * The characteristics added are read only and written once. Do no construct this class.
 * Use the static method add_service to add the chosen Device Information Service characteristics to the server.
 *
 * You can read the specification of the service on the bluetooth website, currently at:
 * https://www.bluetooth.com/specifications/specs/
 * Otherwise search the website for "Device Information Service".
 */
class DeviceInformationService {
public:
    /** System ID */
    struct system_id_t {
        /** Bottom 5 bytes used. */
        uint32_t organizationally_unique_identifier;
        /** Bottom 3 bytes used. */
        uint64_t manufacturer_defined_identifier;
    };

    /** Plug and Play ID */
    struct pnp_id_t {
        /** Specifies where the id is taken from. Currently the only legal values are:
         * 0x01 Bluetooth SIG-assigned Device ID Vendor ID value from the Assigned Numbers document,
         * 0x02 USB Implementer’s Forum assigned Vendor ID value.
         * Other values reserved for future use. */
        uint8_t 	vendor_id_source;
        /** The Vendor IDfield is intended to uniquely identify the vendor of the device. */
        uint16_t 	vendor_id;
        /** The ProductID field is intended to distinguish between different products made by the vendor
         * identified with the Vendor ID field. The vendors themselves manage Product ID field values. */
        uint16_t 	product_id;
        /** The value of the field follows the scheme 0xJJMN for version JJ.M.N
         * (JJ – major version number, M – minor version number, N – sub-minor version number). */
        uint16_t 	product_version;
    };

    /** IEEE 11073-20601 Regulatory Certification Data List */
    struct regulatory_cert_data_list_t {
        /** Regulatory and certification information for the product in a list defined in IEEE 11073-20601.
         *
         * Must conform to IEEE Std 11073-20601-2008 Health Informatics - Personal Health Device Communication
         * Application Profile - Optimized Exchange Protocol version 1.0 or later.
         *
         * @Attention Pointer must be valid and first byte must be equal to the size of the data
         * (one byte less than size of the buffer). */
        const uint8_t *data;
    };

public:
    /** Adds device-specific information into the BLE stack. This must only be called once.
     *
     * @param[in] ble A reference to a BLE object for the underlying controller.
     * @param[in] manufacturers_name The name of the manufacturer of the device.
     * @param[in] model_number The model number that is assigned by the device vendor.
     * @param[in] serial_number The serial number for a particular instance of the device.
     * @param[in] hardware_revision The hardware revision for the hardware within the device.
     * @param[in] firmware_revision The device's firmware version.
     * @param[in] software_revision The device's software version.
     * @param[in] system_id The device's System ID.
     * @param[in] cert_data_list The device's Regulatory Certification Data List.
     * @param[in] pnp_id The device's Plug and Play ID.
     *
     * @note Do not call more than once. Calling this multiple times will create multiple
     * instances of the service which is against the spec.
     */
    static ble_error_t add_service(
        BLE &ble,
        const char *manufacturers_name = nullptr,
        const char *model_number       = nullptr,
        const char *serial_number      = nullptr,
        const char *hardware_revision  = nullptr,
        const char *firmware_revision  = nullptr,
        const char *software_revision  = nullptr,
        const system_id_t *system_id     = nullptr,
        const regulatory_cert_data_list_t *cert_data_list = nullptr,
        const pnp_id_t *pnp_id           = nullptr
    );

private:
    DeviceInformationService() = delete;
    ~DeviceInformationService() = delete;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef BLE_DEVICE_INFORMATION_SERVICE_H */
