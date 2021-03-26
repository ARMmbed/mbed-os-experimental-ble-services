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
import asyncio
from common.fixtures import BoardAllocator, ClientAllocator
from common.device import Device
from os import urandom
from bleak.uuids import uuid16_dict
from bleak import BleakClient
from typing import List, Optional, Any, Mapping
import logging

log = logging.getLogger(__name__)

uuid16_dict = {v: k for k, v in uuid16_dict.items()}

UUID_FOTA_SERVICE = "53880000-65fd-4651-ba8e-91527f06c887"
UUID_BINARY_STREAM_CHAR = "53880001-65fd-4651-ba8e-91527f06c887"
UUID_CONTROL_CHAR = "53880002-65fd-4651-ba8e-91527f06c887"
UUID_STATUS_CHAR = "53880003-65fd-4651-ba8e-91527f06c887"
UUID_VERSION_CHAR = "53880004-65fd-4651-ba8e-91527f06c887"
UUID_FIRMWARE_REVISION_STRING_CHAR = "0000{0:x}-0000-1000-8000-00805f9b34fb".format(
    uuid16_dict.get("Firmware Revision String")
)

FOTA_STATUS_OK = bytearray(b'\x00')
FOTA_STATUS_UPDATE_SUCCESSFUL = bytearray(b'\x01')
FOTA_STATUS_XOFF = bytearray(b'\x02')
FOTA_STATUS_XON = bytearray(b'\x03')
FOTA_STATUS_SYNC_LOST = bytearray(b'\x04')
FOTA_STATUS_UNSPECIFIED_ERROR = bytearray(b'\x05')
FOTA_STATUS_VALIDATION_FAILURE = bytearray(b'\x06')
FOTA_STATUS_INSTALLATION_FAILURE = bytearray(b'\x07')
FOTA_STATUS_OUT_OF_MEMORY = bytearray(b'\x08')
FOTA_STATUS_MEMORY_ERROR = bytearray(b'\x09')
FOTA_STATUS_HARDWARE_ERROR = bytearray(b'\x0a')
FOTA_STATUS_NO_FOTA_SESSION = bytearray(b'\x0b')

FOTA_OP_CODE_START = bytearray(b'\x01')
FOTA_OP_CODE_STOP = bytearray(b'\x02')
FOTA_OP_CODE_COMMIT = bytearray(b'\x03')
FOTA_OP_CODE_SET_XOFF = bytearray(b'\x41')
FOTA_OP_CODE_SET_XON = bytearray(b'\x42')
FOTA_OP_CODE_SET_FRAGMENT_ID = bytearray(b'\x43')

FRAGMENT_SIZE = 128


class DataGenerator:

    def __init__(self, chunk_size):
        self.chunk_size = chunk_size
        self.chunks = []

    def get_chunk_size(self):
        return self.chunk_size

    def get_chunk_n(self, n: int) -> bytearray:
        # See if we've already generated the requested chunk number
        if n < len(self.chunks):
            return self.chunks[n]
        else:
            # We must generate up to the requested chunk number
            while len(self.chunks) < n+1:
                self.chunks.append([])
                self.chunks[len(self.chunks)-1] = bytearray(urandom(self.chunk_size))
            return self.chunks[n]


class StatusNotificationHandler:

    def __init__(self):
        self.status_val = bytearray()
        self.new_status_event = asyncio.Event()

    def handle_status_notification(self, char_handle: int, data: bytearray):
        log.info(f"Status notification: {''.join('{:02x}'.format(x) for x in data)}")
        self.status_val = data
        self.new_status_event.set()

    async def wait_for_status_notification(self):
        await self.new_status_event.wait()
        self.new_status_event.clear()


class FOTASession:

    def __init__(self, board: Device, client: BleakClient):
        self.board = board
        self.client = client
        self.data_generator = DataGenerator(FRAGMENT_SIZE)
        self.handler = StatusNotificationHandler()
        self.fragment_id = 0
        self.rollover_counter = 0

    async def start(self):
        # Subscribe to notifications from the status characteristic
        await self.client.start_notify(UUID_STATUS_CHAR, self.handler.handle_status_notification)

        # Start a FOTA session
        await self.client.write_gatt_char(UUID_CONTROL_CHAR, FOTA_OP_CODE_START, True)

        # Wait for the client to write OK to the status characteristic
        await self.handler.wait_for_status_notification()
        assert self.handler.status_val == FOTA_STATUS_OK
        # FOTA session started

    async def write_n_packets(self, n: int) -> Optional[bytearray]:
        i = (self.rollover_counter * 0xFF) + self.fragment_id
        limit = self.fragment_id + n
        while i < limit:
            # Check for status notification
            if self.handler.new_status_event.is_set():
                self.handler.new_status_event.clear()
                if self.handler.status_val[0] == FOTA_STATUS_SYNC_LOST[0]:
                    log.info(f'Retransmission requested from fragment ID: {self.handler.status_val[1]}')
                return self.handler.status_val

            # Send the next packet
            log.info(f'Writing packet {i}')
            packet = bytearray([self.fragment_id])
            data_chunk = self.data_generator.get_chunk_n(i)
            packet += data_chunk
            await asyncio.wait_for(self.client.write_gatt_char(UUID_BINARY_STREAM_CHAR, packet), timeout=0.2)

            # Check the data received over serial
            lines = self.board.wait_for_output("bsc written: ", timeout=1, assert_timeout=False)
            for line in lines:
                # Skip lines that don't begin with "bsc written: "
                if "bsc written: " not in line:
                    continue
                # Get the data
                data_string = line.split("bsc written: ", 1)[1].strip()
                data = bytearray.fromhex(data_string)
                assert data == data_chunk

            # Roll over the packet ID
            self.fragment_id += 1
            if self.fragment_id >= 256:
                self.fragment_id = 0
                self.rollover_counter += 1

            i = (self.rollover_counter * 0xFF) + self.fragment_id

        return None

    async def wait_for_status_notification(self):
        await self.handler.new_status_event.wait()
        self.handler.new_status_event.clear()
        return self.handler.status_val

    # Sets the fragment ID using a custom op code on the FOTA target side
    # This lets us simulate out-of-sync scenarios
    async def set_remote_fragment_id(self, fragment_id: int):
        command = FOTA_OP_CODE_SET_FRAGMENT_ID + bytearray([fragment_id])
        await self.client.write_gatt_char(UUID_CONTROL_CHAR, command, True)

@pytest.fixture(scope="function")
def board(board_allocator: BoardAllocator):
    board = board_allocator.allocate('FOTA')
    yield board
    board_allocator.release(board)


@pytest.fixture(scope="function")
async def client(client_allocator: ClientAllocator):
    client = await client_allocator.allocate('FOTA')
    yield client
    await client_allocator.release(client)

@pytest.mark.asyncio
async def test_false_start(board, client):
    # Write a random array of bytes to the BSC to generate a false start conditino
    # ie: no ongoing FOTA session
    await client.write_gatt_char(UUID_BINARY_STREAM_CHAR, bytearray(urandom(2)))
    assert await client.read_gatt_char(UUID_STATUS_CHAR) == FOTA_STATUS_NO_FOTA_SESSION


@pytest.mark.asyncio
async def test_data_correctness(board, client):
    session = FOTASession(board, client)
    await session.start()
    await session.write_n_packets(10)

@pytest.mark.asyncio
async def test_out_of_sync(board, client):

    packets_before_sync_lost = 2

    session = FOTASession(board, client)
    await session.start()
    await session.write_n_packets(packets_before_sync_lost)
    # Now simulate a sync lost condition
    session.fragment_id = 50
    await session.write_n_packets(1)
    actual = await session.wait_for_status_notification()
    expected = (FOTA_STATUS_SYNC_LOST + bytearray([packets_before_sync_lost]))
    # We expect a sync lost notification
    assert actual == expected
    # Continue sending out-of-sync packets
    await session.write_n_packets(1)
    actual = await session.wait_for_status_notification()
    # Again, we expect a sync lost notification
    assert actual == expected
    # Resynchronize
    session.fragment_id = packets_before_sync_lost
    await session.write_n_packets(1)
    # Now we expect no issue (timeout on the status notification)
    try:
        actual = await asyncio.wait_for(session.wait_for_status_notification(), timeout=1.0)
        assert not actual
    except asyncio.TimeoutError as e:
        log.info("Status notification timed out as expected")

@pytest.mark.asyncio
async def test_flow_control(board, client):

    packets_before_flow_control = 5
    session = FOTASession(board, client)
    await session.start()
    await session.write_n_packets(packets_before_flow_control)
    # Now simulate a flow control on situation by writing the custom op code
    await client.write_gatt_char(UUID_CONTROL_CHAR, FOTA_OP_CODE_SET_XOFF)
    # We expect to get an XOFF notification now
    status = await session.wait_for_status_notification()
    expected = FOTA_STATUS_XOFF + bytearray([packets_before_flow_control])
    assert status == expected
    # Now try sending a packet to the BSC
    await session.write_n_packets(1)
    status = await session.wait_for_status_notification()
    assert status == expected
    # Tell it to send XON now
    await client.write_gatt_char(UUID_CONTROL_CHAR, FOTA_OP_CODE_SET_XON)
    # We expect to get an XON notification now
    status = await session.wait_for_status_notification()
    expected = FOTA_STATUS_XON + bytearray([packets_before_flow_control])
    assert status == expected
    # Set the session back to the specified fragment ID
    session.fragment_id = status[1]
    # Transfer a few more packets before success is reported
    await session.write_n_packets(5)

@pytest.mark.asyncio
async def test_fragment_id_rollover(board, client):
    session = FOTASession(board, client)
    await session.start()
    await session.write_n_packets(500)