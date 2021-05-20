# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

set(TEST_NAME ble-service-device-information-unittest)

add_executable(${TEST_NAME})

target_include_directories(${TEST_NAME}
    PRIVATE
        .
	${SERVICES_PATH}/DeviceInformation/include
        ${MBED_PATH}/connectivity/FEATURE_BLE/include/ble/gap
)

target_sources(${TEST_NAME}
    PRIVATE
        test_DeviceInformationService.cpp
	${SERVICES_PATH}/DeviceInformation/source/DeviceInformationService.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        mbed-fakes-ble
        mbed-fakes-event-queue
        mbed-headers-base
        mbed-headers-platform
        mbed-headers-connectivity
        gmock_main
)

add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})
