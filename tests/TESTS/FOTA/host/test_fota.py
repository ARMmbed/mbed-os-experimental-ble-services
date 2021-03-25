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
from os import urandom
from bleak.uuids import uuid16_dict
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

    def has_new_status(self):
        return self.new_status_event.is_set()


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

#@pytest.mark.asyncio
# async def test_false_start(board, client):
#     # Write a random array of bytes to the BSC to generate a false start conditino
#     # ie: no ongoing FOTA session
#     await client.write_gatt_char(UUID_BINARY_STREAM_CHAR, bytearray(urandom(2)))
#     assert await client.read_gatt_char(UUID_STATUS_CHAR) == FOTA_STATUS_NO_FOTA_SESSION

@pytest.mark.asyncio
async def test_data_correctness(board, client):
    # Subscribe to notifications from the status characteristic
    handler = StatusNotificationHandler()
    await client.start_notify(UUID_STATUS_CHAR, handler.handle_status_notification)

    # Start a FOTA session
    await client.write_gatt_char(UUID_CONTROL_CHAR, FOTA_OP_CODE_START, True)

    # Wait for the client to write OK to the status characteristic
    await handler.wait_for_status_notification()
    assert handler.status_val == FOTA_STATUS_OK

    # FOTA session started

    # We will test data correctness and fragment ID roll over at the same time
    i = 0
    fragment_id = 0
    rollover_counter = 0
    data_generator = DataGenerator(FRAGMENT_SIZE)
    while i < 512:
        # Check for out of sync notification
        if handler.has_new_status():
            if handler.status_val[0] == FOTA_STATUS_SYNC_LOST[0]:
                requested_fragment_id = handler.status_val[1]
                # Make sure to handle rollover cases
                if requested_fragment_id > fragment_id:
                    rollover_counter -= 1
                fragment_id = requested_fragment_id
                log.info(f'Retransmission requested from fragment ID: {fragment_id}')
            else:
                assert handler.status_val[0] == FOTA_STATUS_OK  # Fail if some other error occurs

        # Send the next packet
        i = (rollover_counter * 0xFF) + fragment_id
        log.info(f'Writing packet {i}')
        packet = bytearray([fragment_id])
        data_chunk = data_generator.get_chunk_n(i)
        packet += data_chunk
        await asyncio.wait_for(client.write_gatt_char(UUID_BINARY_STREAM_CHAR, packet), timeout=0.2)
        await asyncio.sleep(0.05)
        # Roll over the packet ID
        fragment_id += 1
        if fragment_id >= 256:
            fragment_id = 0
            rollover_counter += 1

# TODO out of sync test
# TODO flow control test