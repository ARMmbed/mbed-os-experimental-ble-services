/*
 * Mbed-OS Microcontroller Library
 *
 * [[[cog
 *    import cog, datetime, json, inflection
 *    with open(spec_file, 'r') as f:
 *       spec = json.loads(f.read())
 *       # Save this for later in the global dictionary
 *       globals()['spec'] = spec
 *       cog.outl(f' * Copyright (c) { datetime.date.today().year } { spec["copyright-org"] }')
 * ]]]
 * [[[end]]]
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
 *
 * [[[cog
 *    cog.outl(f' * The basis for this file was automatically generated.\n'
 *             f' * Generation tools Copyright (c) { datetime.date.today().year } Embedded Planet, Inc')
 * ]]]
 * [[[end]]]
 *
 */

//[[[cog
//h_guard = f'{ inflection.underscore(spec["service"]["name"]).upper() }_H'
//cog.outl(f'#ifndef { h_guard }')
//cog.outl(f'#define { h_guard }')
//globals()['h_guard'] = h_guard
//]]]
//[[[end]]]

//[[[cog
//chars = spec["service"]["characteristics"]
//has_custom = False
//# Temporarily append the service name and UUID to the beginning of the list
//chars.insert(0, {'name': 'Base', 'uuid': spec["service"]["uuid"]})
//for c in chars:
//   if len(c['uuid']) > 6:
//      has_custom = True
//      break
//
//if has_custom:
//   cog.outl('namespace uuids { ')
//   cog.outl(f'namespace { spec["service"]["name"] } {{')
//   prefix = '    extern const char '
//   postfix = 'UUID[];'
//   for c in chars:
//      cog.outl(prefix + c['name'] + postfix)
//   cog.outl('}}')
//# Remove the service name from chars
//chars.pop(0)
//]]]
//[[[end]]]

#if BLE_FEATURE_GATT_SERVER

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/gatt/GattCharacteristic.h"

//[[[cog
//cog.outl(f'class { spec["service"]["name"] } {{')
//cog.outl('\npublic:\n')
//cog.outl(f'    { spec["service"]["name"] }(BLE &ble);\n')
//cog.outl(f'    ~{ spec["service"]["name"] }();\n')
//cog.outl(f'    void init();\n')
//cog.outl('protected:\n')
//cog.outl('    BLE &_ble;\n')
//for c in spec["service"]["characteristics"]:
//   cog.outl(f'    GattCharacteristic _{ inflection.underscore(c["name"]) }_char;')
//   cog.outl(f'    { c["type"] } _{ inflection.underscore(c["name"]) };\n')
//]]]
//[[[end]]]

};

#endif /* BLE_FEATURE_GATT_SERVER */

//[[[cog
//cog.outl(f'#endif /* { h_guard } */')
//]]]
//[[[end]]]
