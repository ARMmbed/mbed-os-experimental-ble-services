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

import logging

from bleak import BleakClient, BleakScanner, BleakError

logger = logging.getLogger(__name__)


class Device:

    client: BleakClient = None

    def __init__(self, name: str):
        self.name = name

    async def discoverable(self) -> bool:
        """
        Determine if the given device is discoverable
        :return: True if discoverable, false otherwise
        """
        logger.info(f'Scanning for device named {self.name}...')
        devices = await BleakScanner.discover()
        for device in devices:
            if self.name == device.name:
                self.client = BleakClient(device.address)
                return True
        logger.error(f'Cannot find device named {self.name}')
        return False


async def dut(name):
    """
    Construct a device and attempt to connect to it
    :param name: Name of the Device under Test (DUT)
    :return: If connection succeeds: the device
    """
    device = Device(name)
    attempts = 5
    while attempts > 0:
        try:
            if await device.discoverable() is True:
                connected = await device.client.connect()
                if connected is True:
                    return device
        except BleakError:
            pass
        attempts -= 1
    return None
