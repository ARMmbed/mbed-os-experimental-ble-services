/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2020 Embedded Planet
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
 * limitations under the License
 */

#ifndef MBED_OS_EXPERIMENTAL_BLE_SERVICES_DESCRIPTORS_CHARACTERISTICPRESENTATIONFORMATDESCRIPTOR_H_
#define MBED_OS_EXPERIMENTAL_BLE_SERVICES_DESCRIPTORS_CHARACTERISTICPRESENTATIONFORMATDESCRIPTOR_H_

#include "GattCharacteristic.h"
#include "GattAttribute.h"

/** Length of a presentation format descriptor struct */
#define PRESENTATION_DESC_LEN 7

/**
 * Class encapsulating a GATT Presentation Format Descriptor
 *
 * See Bluetooth Core Specification 5.2, Volume 3, Part G, Section 3.3.3.5
 *
 */
class GattPresentationFormatDescriptor : public GattAttribute
{
    public:

        GattPresentationFormatDescriptor(uint8_t format_type, uint16_t unit = GattCharacteristic::BLE_GATT_UNIT_NONE,
                int8_t exponent = 1, uint8_t namespace_id = 0x01, uint16_t namespace_description = 0x0000) :
                    GattAttribute((const UUID&) UUID(BLE_UUID_DESCRIPTOR_CHAR_PRESENTATION_FORMAT),
                    (uint8_t*) format, PRESENTATION_DESC_LEN, PRESENTATION_DESC_LEN, false)
        {

            /** Populate the format struct */
//          format.gatt_format = format_type;
//          format.exponent = exponent;
//          format.gatt_unit = unit;
//          format.gatt_namespace = namespace_id;
//          format.gatt_nsdesc = namespace_description;

            format[0] = format_type;
            format[1] = exponent;
            memcpy(&format[2], &unit, sizeof(unit));
            format[4] = namespace_id;
            memcpy(&format[5], &namespace_description, sizeof(namespace_description));

        }

        uint8_t get_format() const {
            return format[0];
        }

        uint8_t get_exponent() const {
            return format[1];
        }

        uint16_t get_unit_type() const {
            // TODO do we need to do this? information is encoded in little endian but could be unaligned
            uint16_t retval;
            memcpy(&retval, &format[2], sizeof(uint16_t));
            return retval;
        }

        uint8_t get_namespace_id() const {
            return format[4];
        }

        uint16_t get__description() const {
            uint16_t retval;
            memcpy(&retval, &format[5], sizeof(uint16_t));
            return retval;
        }

    private:

        // Wouldn't it be nice if packing structs was more consistently supported by compilers?
        //struct GattCharacteristic::PresentationFormat_t format;

        // In lieu of using the struct above, packing issues makes us have to use a raw buffer
        uint8_t format[7];
};

#endif /* MBED_OS_EXPERIMENTAL_BLE_SERVICES_DESCRIPTORS_CHARACTERISTICPRESENTATIONFORMATDESCRIPTOR_H_ */
