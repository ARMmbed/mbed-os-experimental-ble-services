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
//chars = spec["service"]["characteristics"]
//s_name = spec["service"]["name"]
//cog.outl(f'#include "{ s_name }.h"\n')
//has_custom = False
//longest_name_len = 0
//# Append the service name and UUID to the beginning of the list
//# This ensures the padding logic below works for the service UUID as well
//chars.insert(0, {'name': 'Base', 'uuid': spec["service"]["uuid"]})
//for c in chars:
//   if len(c['name']) > longest_name_len:
//      longest_name_len = len(c['name'])
//   if len(c['uuid']) > 6:
//      has_custom = True
//
//if has_custom:
//   cog.outl('namespace uuids { ')
//   cog.outl(f'namespace { s_name } {{')
//   # Pad to the longest characteristic name, round up to nearest 4 for tab alignment
//   prefix = '    extern const char '
//   postfix = 'UUID[]'
//   pad_len = ((((longest_name_len + len(prefix) + len(postfix)) // 4) + 1) * 4) - 1
//   for c in chars:
//      cog.outl(f'{{:<{pad_len}}} = "{{}}";'.format((prefix + c['name'] + postfix),
//                                             c['uuid'].upper()))
//   cog.outl('}')
//   cog.outl('}')
//# Remove the service name from chars
//chars.pop(0)
//]]]
//[[[end]]]

#if BLE_FEATURE_GATT_SERVER

//[[[cog
//# Generate the constructor and initializer list
//cog.outl(f'{ s_name }::{ s_name }(BLE &ble) :\n    _ble(ble),')
//for i, c in enumerate(chars):
//   c_name = inflection.underscore(c["name"])
//   cog.outl(f'    _{ c_name }_char(uuids::{ s_name }::{ c["name"] }UUID,')
//   if c["has-variable-length"]:
//      cog.outl(f'        (uint8_t*) &_{ c_name }, { c["length"] }, { c["max-length"] },')
//   else:
//      cog.outl(f'        (uint8_t*) &_{ c_name }, sizeof(_{ c_name }), sizeof(_{ c_name }),')
//   for j, p in enumerate(c["properties"]):
//      cog.out('        ')
//      if not j:
//         cog.out('(')
//      else:
//         cog.out(' ')
//      cog.out(f'GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_{ p }')
//      if j == (len(c["properties"])-1):
//         cog.outl('),')
//      else:
//         cog.outl(' |')
//   cog.outl(f'        nullptr, 0, { "true" if c["has-variable-length"] else "false" }),')
//   cog.out(f'    _{ c_name }(0)')
//   if i != (len(chars) - 1):
//      cog.outl(',')
//   else:
//      cog.outl(' {')
//cog.outl('}')
//
//# Destructor
//cog.outl(f'\n{ s_name }::~{ s_name }() {{ \n}}\n')
//
//# Initialization
//cog.outl(f'void { s_name }::init() {{\n')
//cog.outl('    GattCharacteristic *charTable[] = {')
//for c in chars:
//   c_name = inflection.underscore(c["name"])
//   cog.out(f'                &_{ c_name }_char')
//   if c != chars[-1]:
//      cog.outl(',')
//   else:
//      cog.out('\n')
//cog.outl('    };\n')
//cog.outl(f'    GattService service(uuids::{ s_name }::BaseUUID, charTable, (sizeof(charTable))/sizeof(charTable[0]));')
//]]]
//[[[end]]]

    ble_error_t error = _ble.gattServer().addService(service);
    if(error != BLE_ERROR_NONE) {
        /* Handle error */
    }
}

#endif /* BLE_FEATURE_GATT_SERVER */

