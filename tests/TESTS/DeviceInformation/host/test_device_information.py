# Copyright (c) 2009-2020 Arm Limited
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

import pytest

from common.fixtures import BoardAllocator, ClientAllocator


@pytest.fixture(scope="function")
async def board(board_allocator: BoardAllocator):
    board = await board_allocator.allocate('DeviceInformation')
    yield board
    board_allocator.release(board)


@pytest.fixture(scope="function")
async def client(client_allocator: ClientAllocator):
    client = await client_allocator.allocate('DeviceInformation')
    yield client
    await client_allocator.release(client)


@pytest.mark.asyncio
async def test_device_information_service_values(board, client):
    dev_info_chars = [
        # uuid                                   value
        ('00002a29-0000-1000-8000-00805f9b34fb', bytearray(b'manufacturers_name')),
        ('00002a24-0000-1000-8000-00805f9b34fb', bytearray(b'model_number')),
        ('00002a25-0000-1000-8000-00805f9b34fb', bytearray(b'serial_number')),
        ('00002a27-0000-1000-8000-00805f9b34fb', bytearray(b'hardware_revision')),
        ('00002a26-0000-1000-8000-00805f9b34fb', bytearray(b'firmware_revision')),
        ('00002a28-0000-1000-8000-00805f9b34fb', bytearray(b'software_revision')),
        ('00002a23-0000-1000-8000-00805f9b34fb', bytearray(b'\x01\x00\x00\x00\x00\x02\x00\x00')),
        ('00002a2a-0000-1000-8000-00805f9b34fb', bytearray(b'\x01\x02')),
        ('00002a50-0000-1000-8000-00805f9b34fb', bytearray(b'\x01\x02\x00\x03\x00\x04\x00')),
    ]

    services = await client.get_services()
    instance_number = 0
    for service in services:
        if service.uuid == '0000180a-0000-1000-8000-00805f9b34fb':
            instance_number += 1
            assert len(service.characteristics) == 9

    assert instance_number == 1

    for char in dev_info_chars:
        uuid, expected_value = char
        char_value = await client.read_gatt_char(uuid)
        assert char_value == expected_value
