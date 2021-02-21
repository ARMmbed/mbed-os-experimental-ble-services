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

from common.device import dut
from .utils import UUID_ALERT_LEVEL_CHAR


@pytest.mark.asyncio
@pytest.fixture(scope="function")
async def link_loss():
    return await dut('LinkLoss')


@pytest.mark.asyncio
async def test_read_alert_level_initial_value(link_loss):
    test_output = await link_loss.client.read_gatt_char(UUID_ALERT_LEVEL_CHAR)
    await link_loss.client.disconnect()
    assert test_output == bytearray(b'\x00')


@pytest.mark.asyncio
@pytest.mark.parametrize(
    "test_input,expected",
    [(bytearray(b'\x00'), bytearray(b'\x00')),
     (bytearray(b'\x01'), bytearray(b'\x01')),
     (bytearray(b'\x02'), bytearray(b'\x02'))])
async def test_alert_level_write(link_loss, test_input, expected):
    await link_loss.client.write_gatt_char(UUID_ALERT_LEVEL_CHAR, test_input)
    test_output = await link_loss.client.read_gatt_char(UUID_ALERT_LEVEL_CHAR)
    await link_loss.client.disconnect()
    assert test_output == expected


# TODO: test the alert mechanism
"""
@pytest.mark.asyncio
@pytest.mark.parametrize(
    "test_input,expected",
    [(bytearray(b'\x00'), ""),
     (bytearray(b'\x01'), "Mild Alert"),
     (bytearray(b'\x02'), "High Alert")])
async def test_alert_mechanism(link_loss, test_input, expected):
"""
