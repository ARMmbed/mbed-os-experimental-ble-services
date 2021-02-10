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

#include "ble/BLE.h"
#include "ble/GattServer.h"

#include "ble/gap/ChainableGapEventHandler.h"
#include "ble-service-link-loss/LinkLossService.h"

#include "Events.h"

#include "ble_mocks.h"
#include "events/EventQueue.h"

#include <chrono>

using namespace ble;
using namespace events;
using namespace std::chrono;
using namespace std::literals::chrono_literals;

using ::testing::Property;
using ::testing::Values;

struct EventHandlerMock : LinkLossService::EventHandler {
    MOCK_METHOD(void, on_alert_requested, (LinkLossService::AlertLevel), (override));
    MOCK_METHOD(void, on_alert_end, (), (override));
};

class TestLinkLossService : public testing::Test {
protected:
    BLE *ble;
    events::EventQueue event_queue;
    ChainableGapEventHandler chainable_gap_event_handler;

    std::unique_ptr<LinkLossService> link_loss_service;

    void SetUp()
    {
        ble = &BLE::Instance();
        GattServer &server = ble->gattServer();

        link_loss_service = std::make_unique<LinkLossService>(*ble, event_queue, chainable_gap_event_handler);
    }

    void TearDown()
    {
        ble::delete_mocks();
    }
};

class TestLinkLossServiceEvents : public TestLinkLossService,
                                  public testing::WithParamInterface<LinkLossService::AlertLevel> {
protected:
    EventHandlerMock event_handler_mock;

    void SetUp() {
        TestLinkLossService::SetUp();

        link_loss_service->init();

        link_loss_service->set_event_handler(&event_handler_mock);
    }

    void TearDown() {
        TestLinkLossService::TearDown();
    }

    void simulate_connection_event(ble_error_t status)
    {
        const uint8_t  peer_addr_bytes[] = {0xfb, 0xdd, 0x62, 0x03, 0x04, 0xd8};
        const uint8_t local_addr_bytes[] = {0x4d, 0xc7, 0x92, 0x0e, 0x51, 0xba};

        connection_handle_t connectionHandle = 0;
        connection_role_t ownRole = connection_role_t::PERIPHERAL;
        const peer_address_type_t peerAddressType = peer_address_type_t::PUBLIC;
        const address_t peerAddress(peer_addr_bytes);
        const address_t localResolvablePrivateAddress(local_addr_bytes);
        const address_t peerResolvablePrivateAddress(peer_addr_bytes);
        conn_interval_t connectionInterval = conn_interval_t(50);
        slave_latency_t connectionLatency = slave_latency_t::max();
        supervision_timeout_t supervisionTimeout = supervision_timeout_t(100);
        uint16_t masterClockAccuracy = 100;

        ConnectionCompleteEvent connection_complete_event(
                status,
                connectionHandle,
                ownRole,
                peerAddressType,
                peerAddress,
                localResolvablePrivateAddress,
                peerResolvablePrivateAddress,
                connectionInterval,
                connectionLatency,
                supervisionTimeout,
                masterClockAccuracy
        );

        chainable_gap_event_handler.onConnectionComplete(connection_complete_event);
    }

    void simulate_disconnection_event(disconnection_reason_t reason)
    {
        connection_handle_t connectionHandle = 0;

        DisconnectionCompleteEvent disconnection_complete_event(
                connectionHandle,
                reason
        );

        chainable_gap_event_handler.onDisconnectionComplete(disconnection_complete_event);
    }

    GattAuthCallbackReply_t simulate_data_written_event(
            const uint8_t *data,
            uint16_t len,
            uint16_t offset = 0,
            GattAuthCallbackReply_t authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS)
    {
        connection_handle_t connectionHandle = 0;
        GattServerMock::characteristic_t alert_level_char = gatt_server_mock().services[0].characteristics[0];

        GattWriteAuthCallbackParams write_request {
                connectionHandle,
                alert_level_char.value_handle,
                offset,
                len,
                data,
                authorizationReply
        };

        alert_level_char.write_cb(&write_request);

        return write_request.authorizationReply;
    }
};

TEST_F(TestLinkLossService, constructor)
{
    // The link loss service should not be NULL
    ASSERT_TRUE(link_loss_service);
}

TEST_F(TestLinkLossService, init)
{
    // Retrieve the fake gatt server API
    GattServer &server = ble->gattServer();

    // A service with uuid=0x1803 should be added to the gatt server
    EXPECT_CALL(gatt_server_mock(), addService(Property(&GattService::getUUID, GattService::UUID_LINK_LOSS_SERVICE)))
            .Times(1);

    // Initialize the link loss service
    link_loss_service->init();

    // Capture the link loss service registered in gatt server
    auto service = gatt_server_mock().services[0];

    // The link loss service should contain 1 characteristic for the alert level
    ASSERT_EQ(service.characteristics.size(), 1);

    // Capture the alert level characteristic added to the link loss service
    auto characteristic = service.characteristics[0];

    // The alert level characteristic should have a uuid=0x2A06
    ASSERT_EQ(characteristic.uuid, GattCharacteristic::UUID_ALERT_LEVEL_CHAR);

    // The alert level characteristic should be readable and writable
    ASSERT_TRUE(characteristic.properties &
               (GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE));

    // The write authorisation callback for the alert level characteristic should not be NULL
    ASSERT_TRUE(characteristic.write_cb);
}

TEST_F(TestLinkLossServiceEvents, disconnection_reconnection)
{
    // Set the alert timeout to 1 min
    minutes alert_timeout = minutes(1);
    link_loss_service->set_alert_timeout(alert_timeout);

    // Set the alert level to "HIGH ALERT"
    link_loss_service->set_alert_level(LinkLossService::AlertLevel::HIGH_ALERT);

    // Simulate a clean connection event
    simulate_connection_event(BLE_ERROR_NONE);

    // The device should start alerting with an alert level of "HIGH ALERT"
    EXPECT_CALL(event_handler_mock, on_alert_requested(LinkLossService::AlertLevel::HIGH_ALERT));

    // Simulate a disconnection event due to connection timeout
    simulate_disconnection_event(disconnection_reason_t::CONNECTION_TIMEOUT);

    // Dispatch events for 59999 ms
    event_queue.dispatch(59999);

    // Get the number of events in the queue before reconnection
    size_t initial_queue_size = event_queue.size();

    // The alert should end due to reconnection
    EXPECT_CALL(event_handler_mock, on_alert_end())
            .Times(1);

    // Simulate a clean connection event
    simulate_connection_event(BLE_ERROR_NONE);

    // The reconnection should have cancelled the pending timeout
    EXPECT_EQ(event_queue.size(), initial_queue_size - 1);

    // The alert should not end in 1 ms due to the alert timeout
    EXPECT_CALL(event_handler_mock, on_alert_end())
            .Times(0);

    // Dispatch events for a further 1 ms
    event_queue.dispatch(1);
}

TEST_F(TestLinkLossServiceEvents, disconnection_no_timeout)
{
    // Set the alert timeout to 0
    milliseconds alert_timeout = milliseconds (0);
    link_loss_service->set_alert_timeout(alert_timeout);

    // Set the alert level
    link_loss_service->set_alert_level(LinkLossService::AlertLevel::HIGH_ALERT);

    // Simulate a clean connection event
    simulate_connection_event(BLE_ERROR_NONE);

    // Get the number of events in the queue before reconnection
    size_t initial_queue_size = event_queue.size();

    // The device should start alerting with an alert level of "HIGH ALERT"
    EXPECT_CALL(event_handler_mock, on_alert_requested(LinkLossService::AlertLevel::HIGH_ALERT));

    // Simulate a disconnection event due to connection timeout
    simulate_disconnection_event(disconnection_reason_t::CONNECTION_TIMEOUT);

    // No callback should be registered in the event queue because the alert timeout is 0
    ASSERT_EQ(event_queue.size(), initial_queue_size);
}

TEST_F(TestLinkLossServiceEvents, data_written_invalid)
{
    const uint8_t data = static_cast<const uint8_t>(LinkLossService::AlertLevel::HIGH_ALERT) + 1;
    uint16_t len = sizeof(data);

    // Simulate a data written event to set the alert level to an invalid value
    GattAuthCallbackReply_t authorisationReply = simulate_data_written_event(&data, len);

    // The alert level should be equal to the initial value of "NO ALERT"
    ASSERT_EQ(link_loss_service->get_alert_level(), LinkLossService::AlertLevel::NO_ALERT);

    // The write authorisation reply in the write request should be OUT_OF_RANGE
    ASSERT_EQ(authorisationReply, GattAuthCallbackReply_t::AUTH_CALLBACK_REPLY_ATTERR_OUT_OF_RANGE);
}

TEST_P(TestLinkLossServiceEvents, connection)
{
    LinkLossService::AlertLevel alert_level = GetParam();

    milliseconds alert_timeout = milliseconds (60000);
    link_loss_service->set_alert_timeout(alert_timeout);

    link_loss_service->set_alert_level(alert_level);

    simulate_connection_event(BLE_ERROR_NONE);

    size_t cardinality = 0;
    if ((alert_level == LinkLossService::AlertLevel::MILD_ALERT) |
        (alert_level == LinkLossService::AlertLevel::HIGH_ALERT)) {
        cardinality = 1;
    }

    EXPECT_CALL(event_handler_mock, on_alert_requested(alert_level))
         .Times(cardinality);

    simulate_disconnection_event(disconnection_reason_t::CONNECTION_TIMEOUT);

    size_t initial_event_queue_size = event_queue.size();

    EXPECT_CALL(event_handler_mock, on_alert_end())
         .Times(cardinality);

    simulate_connection_event(BLE_ERROR_NONE);

    EXPECT_EQ(event_queue.size(), initial_event_queue_size - cardinality);
}

TEST_P(TestLinkLossServiceEvents, disconnection)
{
    LinkLossService::AlertLevel alert_level = GetParam();

    milliseconds alert_timeout = milliseconds(60000);
    link_loss_service->set_alert_timeout(alert_timeout);

    link_loss_service->set_alert_level(alert_level);

    simulate_connection_event(BLE_ERROR_NONE);

    size_t cardinality = 0;
    if ((alert_level == LinkLossService::AlertLevel::MILD_ALERT) |
        (alert_level == LinkLossService::AlertLevel::HIGH_ALERT)) {
        cardinality = 1;
    }

    EXPECT_CALL(event_handler_mock, on_alert_requested(alert_level))
         .Times(cardinality);

    simulate_disconnection_event(disconnection_reason_t::CONNECTION_TIMEOUT);

    event_queue.dispatch(59999);

    EXPECT_CALL(event_handler_mock, on_alert_end())
         .Times(cardinality);

    event_queue.dispatch(1);
}

TEST_P(TestLinkLossServiceEvents, data_written)
{
    LinkLossService::AlertLevel alert_level = GetParam();

    const uint8_t data = static_cast<const uint8_t>(alert_level);
    uint16_t len = sizeof(data);

    simulate_data_written_event(&data, len);

    ASSERT_EQ(link_loss_service->get_alert_level(), alert_level);
}

INSTANTIATE_TEST_SUITE_P(Expected, TestLinkLossServiceEvents,
                         Values(LinkLossService::AlertLevel::NO_ALERT,
                                LinkLossService::AlertLevel::MILD_ALERT,
                                LinkLossService::AlertLevel::HIGH_ALERT));