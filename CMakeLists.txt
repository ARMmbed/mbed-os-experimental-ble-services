cmake_minimum_required(VERSION 3.12)
project(ble-services)

option(enable_coverage_data "Enable Coverage data" OFF)

if (enable_coverage_data)
        find_program(XCRUN_PATH xcrun)
        if (XCRUN_PATH)
                execute_process (
                        COMMAND sh -c "xcrun -find llvm-cov"
                        OUTPUT_VARIABLE LLVM_COV_PATH
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                )
                execute_process (
                        COMMAND sh -c "xcrun -find llvm-profdata"
                        OUTPUT_VARIABLE LLVM_PROFDATA_PATH
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                )
        else ()
                find_program(LLVM_COV_PATH llvm-cov)
                find_program(LLVM_PROFDATA_PATH llvm-profdata)
                if(NOT LLVM_COV_PATH)
                        message(FATAL_ERROR "llvm-cov not found! Aborting.")
                endif ()
                if(NOT LLVM_PROFDATA_PATH)
                        message(FATAL_ERROR "llvm-profdata not found! Aborting.")
                endif ()
        endif ()
endif ()

set(CMAKE_CXX_STANDARD 14)

option(ENABLE_RTTI "add rtti flag")
option(ENABLE_EXCEPTIONS "add exceptions flag")

add_library(ble-stubs 
        tests/stubs/include/ble/BLE.h
        tests/stubs/include/ble/GattServer.h

        tests/stubs/include/ble/common/BLETypes.h
        tests/stubs/include/ble/common/Bounded.h
        tests/stubs/include/ble/common/CallChainOfFunctionPointersWithContext.h
        tests/stubs/include/ble/common/ChainableEventHandler.h
        tests/stubs/include/ble/common/Duration.h
        tests/stubs/include/ble/common/FunctionPointerWithContext.h
        tests/stubs/include/ble/common/SafeBool.h
        tests/stubs/include/ble/common/SafeEnum.h
        tests/stubs/include/ble/common/UUID.h
        tests/stubs/include/ble/common/blecommon.h

        tests/stubs/include/ble/gap/Types.h

        tests/stubs/include/ble/gatt/ChainableGattServerEventHandler.h
        tests/stubs/include/ble/gatt/GattAttribute.h
        tests/stubs/include/ble/gatt/GattCallbackParamTypes.h
        tests/stubs/include/ble/gatt/GattCharacteristic.h
        tests/stubs/include/ble/gatt/GattService.h

        tests/stubs/include/platform/Span.h
        tests/stubs/include/platform/mbed_assert.h
        tests/stubs/include/platform/mbed_preprocessor.h
        tests/stubs/include/platform/mbed_toolchain.h

        tests/stubs/source/BLE.cpp tests/stubs/source/GattServer.cpp)

target_include_directories(ble-stubs PUBLIC tests/stubs/include)

set(BLE_SERVICES_SOURCES
        include/mbed-ble-services/dummy_service.h
        source/dummy_service.cpp
)

# Append abs path to BLE_SERVICES_SOURCES
set(BLE_SERVICES_SOURCES_ABS_PATH ${BLE_SERVICES_SOURCES})
list(TRANSFORM BLE_SERVICES_SOURCES_ABS_PATH PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_library(ble-services ${BLE_SERVICES_SOURCES})

target_include_directories(ble-services PUBLIC tests/stubs/include)
target_include_directories(ble-services PUBLIC include)

if(ENABLE_EXCEPTIONS)
	target_compile_options(ble-services PUBLIC -fexceptions)
else()
	target_compile_options(ble-services PUBLIC -fno-exceptions)
endif()
if (ENABLE_RTTI)
	target_compile_options(ble-services PUBLIC -frtti)
else()
	target_compile_options(ble-services PUBLIC -fno-rtti)
endif()

target_compile_definitions(ble-services PUBLIC MBED_CONF_MBED_TRACE_FEA_IPV6=0 YOTTA_CFG)
if (enable_coverage_data)
        target_compile_options(ble-services PUBLIC -fprofile-instr-generate -fcoverage-mapping)
endif()

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif ()

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(test_app
        tests/source/dummy_service_test.cpp
        tests/mocks/GattServerMock.h
        tests/mocks/GattServerMock.cpp)

include_directories(test_app ${CMAKE_SOURCE_DIR})
include_directories(test_app tests)

target_link_libraries(test_app gmock_main)
target_link_libraries(test_app ble-services)
target_link_libraries(test_app ble-stubs)

if (enable_coverage_data)
        set(LLVM_PROFILE_RAW "ble-services_coverage.profraw" CACHE FILEPATH "LLVM Profile File")
        set(LLVM_PROFILE_DATA "ble-services_coverage.profdata" CACHE FILEPATH "LLVM Profile Output")
        target_link_options(test_app PUBLIC -fprofile-instr-generate)
endif()

add_test(NAME ble-services_test COMMAND test_app)

# Add coverage reporting command
if (enable_coverage_data)
        set_tests_properties(ble-services_test PROPERTIES 
                ENVIRONMENT LLVM_PROFILE_FILE=${LLVM_PROFILE_RAW})
        
        add_custom_command(OUTPUT ${LLVM_PROFILE_RAW}
        DEPENDS test_app 
        COMMAND LLVM_PROFILE_FILE=${LLVM_PROFILE_RAW} ./test_app)

        add_custom_command(OUTPUT ${LLVM_PROFILE_DATA}
        DEPENDS ${LLVM_PROFILE_RAW}
        COMMAND ${LLVM_PROFDATA_PATH} merge -sparse ${LLVM_PROFILE_RAW} -o ${LLVM_PROFILE_DATA})

        add_custom_target(ble-services_coverage_summary
        DEPENDS ${LLVM_PROFILE_DATA}
        COMMAND ${LLVM_COV_PATH} report ./test_app -instr-profile=${LLVM_PROFILE_DATA} ${BLE_SERVICES_SOURCES_ABS_PATH})

        add_custom_target(ble-services_coverage_report
        DEPENDS ${LLVM_PROFILE_DATA}
        COMMAND ${LLVM_COV_PATH} show ./test_app -format=html -output-dir=coverage/ -instr-profile=${LLVM_PROFILE_DATA} ${BLE_SERVICES_SOURCES_ABS_PATH})
endif ()