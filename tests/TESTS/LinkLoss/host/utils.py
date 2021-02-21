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

from bleak.uuids import uuid16_dict

uuid16_dict = {v: k for k, v in uuid16_dict.items()}

UUID_LINK_LOSS_SERVICE = "0000{0:x}-0000-1000-8000-00805f9b34fb".format(
    uuid16_dict.get("Link Loss")
)
UUID_ALERT_LEVEL_CHAR  = "0000{0:x}-0000-1000-8000-00805f9b34fb".format(
    uuid16_dict.get("Alert Level")
)
