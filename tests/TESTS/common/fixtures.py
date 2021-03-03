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
# limitations under the License.

import mbed_lstools
import platform
import logging
import pytest

from typing import List, Optional, Any, Mapping
from bleak import BleakClient, BleakScanner, BleakError
from mbed_flasher.flash import Flash
from .serial_connection import SerialConnection
from .serial_device import SerialDevice

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def platforms(request):
    if request.config.getoption('platforms'):
        return request.config.getoption('platforms').split(',')
    else:
        return [
            'DISCO_L475VG_IOT01A',
            'NRF52840_DK'
        ]


@pytest.fixture(scope="session")
def binaries(request):
    if request.config.getoption('binaries'):
        platform_and_binaries = request.config.getoption('binaries').split(',')
        result = {}
        for pb in platform_and_binaries:
            pb = pb.split('=')
            result[pb[0]] = pb[1]
        return result
    return {}


@pytest.fixture(scope="session")
def serial_inter_byte_delay(request):
    if request.config.getoption('serial_inter_byte_delay'):
        return float(request.config.getoption('serial_inter_byte_delay'))
    return None


@pytest.fixture(scope="session")
def serial_baudrate(request):
    if request.config.getoption('serial_baudrate'):
        return int(request.config.getoption('serial_baudrate'))
    return 115200


@pytest.fixture(scope="session")
def command_delay(request):
    if request.config.getoption('command_delay'):
        return float(request.config.getoption('command_delay'))
    return float(0)


class BoardAllocation:
    def __init__(self, description: Mapping[str, Any]):
        self.description = description
        self.ble_device = None  # type: Optional[SerialDevice]
        self.flashed = False


class BoardAllocator:
    def __init__(self, platforms_supported: List[str], binaries: Mapping[str, str], serial_inter_byte_delay: float,
                 baudrate: int, command_delay: float):
        mbed_ls = mbed_lstools.create()
        boards = mbed_ls.list_mbeds(filter_function=lambda m: m['platform_name'] in platforms_supported)
        self.board_description = boards
        self.binaries = binaries
        self.allocation = []  # type: List[BoardAllocation]
        self.flasher = None
        self.serial_inter_byte_delay = serial_inter_byte_delay
        self.baudrate = baudrate
        self.command_delay = command_delay
        for desc in boards:
            self.allocation.append(BoardAllocation(desc))

    def allocate(self, name: str = None) -> Optional[SerialDevice]:
        for alloc in self.allocation:
            if alloc.ble_device is None:
                # Flash if a binary is provided and the board hasn't been flashed yet
                platform = alloc.description['platform_name']
                binary = self.binaries.get(platform)
                if alloc.flashed is False and binary:
                    if self.flasher is None:
                        self.flasher = Flash()
                    self.flasher.flash(build=binary, target_id=alloc.description["target_id"])
                    alloc.flashed = True

                # Create the serial connection
                connection = SerialConnection(
                    port=alloc.description["serial_port"],
                    baudrate=self.baudrate,
                    inter_byte_delay=self.serial_inter_byte_delay
                )
                connection.open()

                # Create the serial device
                serial_device = SerialDevice(connection, name)
                serial_device.reset(duration=1)
                serial_device.flush(1)

                # Create the SerialDevice
                alloc.ble_device = serial_device

                return alloc.ble_device
        return None

    def release(self, ble_device: SerialDevice) -> None:
        for alloc in self.allocation:
            if alloc.ble_device == ble_device and alloc.ble_device is not None:
                # Stop activities
                alloc.ble_device.stop()
                alloc.ble_device.serial.close()

                # Cleanup
                alloc.ble_device = None


@pytest.fixture(scope="session")
def board_allocator(
        platforms: List[str],
        binaries: Mapping[str, str],
        serial_inter_byte_delay: float,
        serial_baudrate: int,
        command_delay: float
):
    yield BoardAllocator(platforms, binaries, serial_inter_byte_delay, serial_baudrate, command_delay)


class ClientAllocator:

    def __init__(self):
        self.client = None  # type: Optional[BleakClient]

    async def allocate(self, name: str) -> Optional[BleakClient]:
        if self.client is None:
            devices = await BleakScanner.discover()
            for d in devices:
                if name == d.name:
                    self.client = BleakClient(d)
                    attempts = 5
                    while attempts > 0:
                        try:
                            connected = await self.client.connect()
                            if connected is True:
                                return self.client
                        except BleakError:
                            pass
                        attempts -= 1
        return None

    async def release(self, client: BleakClient) -> None:
        if self.client == client and self.client is not None:
            if await client.is_connected():
                attempts = 5
                while attempts > 0:
                    try:
                        disconnected = await client.disconnect()
                        if disconnected is True:
                            self.client = None
                            return
                    except BleakError:
                        pass
                    attempts -= 1
            else:
                if platform.system() == "Darwin":
                    await client.connect()
                    await self.release(client)


@pytest.fixture(scope="session")
def client_allocator():
    yield ClientAllocator()
