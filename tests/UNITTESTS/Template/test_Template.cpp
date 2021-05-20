/*
 * Copyright (c) 2020, Arm Limited and affiliates.
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
 * limitations under the License.
 */

#include "gtest/gtest.h"

/* these are real mbed-os headers but the implementation will be provided by mocks */
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"
#include "ble/GattClient.h"
#include "ble/SecurityManager.h"

/* this provides all the ble mocks inside the fake BLE instance */
#include "ble_mocks.h"
/* this is a fake event queue that avoids mbed-os dependencies and allows manual dispatch */
#include "events/EventQueue.h"

using namespace std::chrono_literals;

/* This test does not test anything, you may use it as a template for your unit tests.
 * It shows all the elements you need to use mocks for all the ble APIS. */

class TestTemplate : public testing::Test {
protected:
    void SetUp()
    {
        /* this call uses ble::init_mocks() to initialises the mocks */
        ble = &BLE::Instance();
    }

    void TearDown()
    {
        /* remember you must call this at the end of the test if you have any expectations set */
        ble::delete_mocks();
    }

    BLE* ble;
    events::EventQueue queue;
};

TEST_F(TestTemplate, reset)
{
    /* these are the user facing APIs */
    Gap &gap = ble->gap();
    GattClient &client = ble->gattClient();
    GattServer &server = ble->gattServer();
    SecurityManager &sm = ble->securityManager();

    /* they in turn call the implementations which are mocked and can have expectations set on them */
    EXPECT_CALL(ble::gap_mock(), reset());
    EXPECT_CALL(ble::gatt_client_mock(), reset());
    EXPECT_CALL(ble::gatt_server_mock(), reset(&server)); /* this reset contains pointer to user API */
    EXPECT_CALL(ble::security_manager_mock(), reset());

    /* calls are made on real APIs but will fulfill expectations on the mocks as that's what they call in the end */
    gap.reset();
    client.reset();
    server.reset();
    sm.reset();

    /* you use the fake event queue similarly to the real one */
    queue.call_in(100ms, [](){ printf("100ms have passed\r\n"); });
    queue.call([](){ printf("I will be called first\r\n"); });
    queue.call_in(10ms, [](){ printf("10ms have passed\r\n"); });
    /* events will be dispatched in the correct order */

    int handle = queue.call([](){ printf("I will not be called\r\n"); });
    queue.cancel(handle);

    /* you have to dispatch the queue yourself, you can decide how many milliseconds have passed
     * (call with no arguments to dispatch all events) */
    queue.dispatch(10);
    queue.dispatch(90);
}
