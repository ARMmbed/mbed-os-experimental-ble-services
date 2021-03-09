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
import platform

from common.fixtures import BoardAllocator, ClientAllocator
from bleak.uuids import uuid16_dict

uuid16_dict = {v: k for k, v in uuid16_dict.items()}

UUID_ALERT_LEVEL_CHAR = "0000{0:x}-0000-1000-8000-00805f9b34fb".format(
    uuid16_dict.get("Alert Level")
)

UUID_DISCONNECTION_REASON_CHAR = "f43620d0-779d-11eb-9439-0242ac130002"

CONNECTION_TIMEOUT = bytearray(b'\x08')

NO_ALERT   = bytearray(b'\x00')
MILD_ALERT = bytearray(b'\x01')
HIGH_ALERT = bytearray(b'\x02')

alert_timeout = 5


@pytest.fixture(scope="function")
def board(board_allocator: BoardAllocator):
    board = board_allocator.allocate('LinkLoss')
    yield board
    board_allocator.release(board)


@pytest.fixture(scope="function")
async def client(client_allocator: ClientAllocator):
    client = await client_allocator.allocate('LinkLoss')
    yield client
    await client_allocator.release(client)


@pytest.mark.asyncio
async def test_read_alert_level_initial_value(board, client):
    assert await client.read_gatt_char(UUID_ALERT_LEVEL_CHAR) == NO_ALERT


@pytest.mark.asyncio
@pytest.mark.parametrize(
    "alert_level", [NO_ALERT, MILD_ALERT, HIGH_ALERT])
async def test_alert_level_write(board, client, alert_level):
    await client.write_gatt_char(UUID_ALERT_LEVEL_CHAR, alert_level)
    assert await client.read_gatt_char(UUID_ALERT_LEVEL_CHAR) == alert_level


@pytest.mark.asyncio
@pytest.mark.parametrize(
    "alert_level,alert_message", [(MILD_ALERT, "Mild Alert!"), (HIGH_ALERT, "High Alert!")])
async def test_alert_mechanism(board, client, alert_level, alert_message):
    await client.write_gatt_char(UUID_ALERT_LEVEL_CHAR, alert_level)
    await client.write_gatt_char(UUID_DISCONNECTION_REASON_CHAR, CONNECTION_TIMEOUT)
    # On Windows, we need to trigger a normal disconnection to prevent Bleak from trying to automatically reconnect
    # This does not affect the application running on the device since we are already disconnected
    if platform.system() == "Windows":
        await client.disconnect()
    board.wait_for_output(alert_message, timeout=10)
    board.wait_for_output("Alert ended", timeout=alert_timeout)


@pytest.mark.asyncio
@pytest.mark.parametrize(
    "alert_level,alert_message", [(MILD_ALERT, "Mild Alert!"), (HIGH_ALERT, "High Alert!")])
async def test_disconnection_reconnection(board, client, alert_level, alert_message):
    await client.write_gatt_char(UUID_ALERT_LEVEL_CHAR, alert_level)
    await client.write_gatt_char(UUID_DISCONNECTION_REASON_CHAR, CONNECTION_TIMEOUT)
    # On Windows, we need to trigger a normal disconnection to prevent Bleak from trying to automatically reconnect
    # This does not affect the application running on the device since we are already disconnected
    if platform.system() == "Windows":
        await client.disconnect()
    board.wait_for_output(alert_message, timeout=10)
    await client.connect()
    board.wait_for_output("Alert ended", timeout=10)
