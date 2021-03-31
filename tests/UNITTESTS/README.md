# Unit testing

The services provided are covered by unit tests.
We use the [GoogleTest](https://github.com/google/googletest) framework with the [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) runner.

Please add a test suite to your PR when adding a new service.
Feel free to use any framework as long as it integrates with our CTest runner, but to take advantage of mocks provided by Mbed OS you will have to use GoogleTest.

## Test code structure
Each test suite should be named after the service under test (SUT) and contain two files: a C++ test file and a `CMakeLists.txt`.
For example, the directory tree for the LinkLoss test suite is shown below.

```
LinkLoss/
├─── CMakeLists.txt
└─── test_LinkLossService.cpp
```

For the `CMakeLists.txt`, you may use the following template.
Please change the "your service" values to the actual name of your service.

```cmake
# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.0.2)

# change your-service to the name of your service
set(TEST_NAME ble-service-your-service-unittest)

add_executable(${TEST_NAME})

target_include_directories(${TEST_NAME}
    PRIVATE
        .
        # change the path to where your includes are
        ${SERVICES_PATH}/YourService/include
)

target_sources(${TEST_NAME}
    PRIVATE
        # replace with the source file you have your unit test in
        test_YourService.cpp
        # replace the source file with the one that compiles your service
        ${SERVICES_PATH}/YourService/source/YourService.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        # add any stubs, headers or mocks you need here
        # you will most likely want to include:
        # mbed-os-fakes-ble
        gmock_main
)

target_compile_definitions(${TEST_NAME}
     PUBLIC
        # CMake does not generate configuration options;
        # if your service has an mbed_lib.json, you must define the configuration parameters here
        BLE_FEATURE_GATT_SERVER=1
)

add_test(NAME "${TEST_NAME}" COMMAND ${TEST_NAME})
```

Please add your test suite as a subdirectory in the top-level `CMakeLists.txt` by appending the following line to it:

```cmake
add_subdirectory(YourService)
```

## Unit testing with GoogleTest

### Stubs, mocks and fakes in Mbed OS

To make it easier to test your service, Mbed OS provides some ready-made CMake libraries you can add in your `target_link_libraries` section (see template above). 
Libraries can be object files that contain stubs, mocks and fakes but also sets of include paths - this way you can gain access to normal headers from Mbed OS.

All the libraries available are in the `CMakeLists.txt` files in `mbed-os/UNITTESTS/stubs` and `mbed-os/UNITTESTS/fakes`.

The most common ones you might need are `mbed-os-fakes-ble` and `mbed-os-fakes-event-queue`.

#### mbed-os-fakes-ble

This library provides a fake BLE implementation that uses mocks instead of real BLE components for `Gap`, `GattServer`, `GattClient`, `SecurityManager`.

There is no need to initialise a fake BLE instance; it is ready to go and can be used just like a normal BLE instance:

```c++
BLE *ble = &BLE::Instance();
```

This call also initialises mocks. 
Do no cache the BLE instance pointer, or pointer to `GAP`, `GattServer` etc. between tests. 
You must get the instance fresh at the start of the test.

You can retrieve all the BLE APIs from the instance just like with a normal one:

```c++
Gap &gap = ble->gap();
GattClient &client = ble->gattClient();
GattServer &server = ble->gattServer();
SecurityManager &sm = ble->securityManager();
```

Whenever an API call is made, the implementation will be called. 
These are replaced in the fake BLE with google mocks.
This means you can set expectations on them. 

```c++
EXPECT_CALL(ble::gap_mock(), reset());
```

This will set up an expectation that at some point during the test the `Gap::reset` method will be called.

The way GoogleTest works means that if you set expectations on your mocks they must be destroyed at the end of each test. 
This is done through the fake BLE instance special method:

```c++
ble::delete_mocks();
```

#### mbed-os-fakes-event-queue

This is a fake event queue that doesn't bring in any dependencies from mbed-os. 
Its API is simplified and it only offers limited functionality.

If you choose to use it you must not also include a library that brings in real headers for the event queue as they would conflict.

The API calls supported are for simple calls `call`, `call_in` and the `cancel` method.

The event queue is not run in real time and must be progressed manually. 
You may use `dispatch(int milliseconds)` and `dispatch_forever()` to process events in the queue. 
This way you can simulate the passage of time in your test.

## Building and running unit tests

1. Run the bootstrap process:

    ```shell
    ../../scripts/bootstrap.sh
    ```
   
1. Build unit tests with CMake:

    ```shell
    ./build.sh
    ```

1. Run unit tests:

    ```shell
    ./run.sh
    ```

   Unit tests depend on Mbed OS to provide stubs and mocks.
   As such, it is cloned into the dependencies folder during the bootstrap process and symlinked here.
