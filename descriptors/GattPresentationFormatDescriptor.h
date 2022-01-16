/* mbed Microcontroller Library
 * Copyright (c) 2020-2021 Embedded Planet, Inc
 * Copyright (c) 2020-2021 ARM Limited
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

#ifndef GATT_PRESENTATION_FORMAT_DESCRIPTOR_H_
#define GATT_PRESENTATION_FORMAT_DESCRIPTOR_H_

#include "GattCharacteristic.h"
#include "GattAttribute.h"

/** Length of a presentation format descriptor struct */
#define GATT_PRESENTATION_FORMAT_DESCRIPTOR_LEN 7

/**
 * Class encapsulating a GATT Presentation Format Descriptor
 */
class GattPresentationFormatDescriptor: public GattAttribute {

public:

    GattPresentationFormatDescriptor(uint8_t format_type, uint16_t unit =
            GattCharacteristic::BLE_GATT_UNIT_NONE, int8_t exponent = 1,
            uint8_t namespace_id = 0x01,
            uint16_t namespace_description = 0x0000) :
            GattAttribute(
                    (const UUID&) UUID(
                            BLE_UUID_DESCRIPTOR_CHAR_PRESENTATION_FORMAT),
                    (uint8_t*) format, GATT_PRESENTATION_FORMAT_DESCRIPTOR_LEN,
                    GATT_PRESENTATION_FORMAT_DESCRIPTOR_LEN, false) {

        format[0] = format_type;
        format[1] = exponent;
        format[2] = (unit & 0xFF00) >> 8;
        format[3] = (unit & 0x00FF);
        format[4] = namespace_id;
        format[5] = (namespace_description & 0xFF00) >> 8;
        format[6] = (namespace_description & 0x00FF);

    }

private:

    uint8_t format[7];
};

#endif /* GATT_PRESENTATION_FORMAT_DESCRIPTOR_H_ */
