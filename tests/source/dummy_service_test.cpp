// Dummy test to setup CMake 

#include "mocks/GattServerMock.h"
#include "gtest/gtest.h"

#include "mbed-ble-services/dummy_service.h"

#include "ble/BLE.h"

// Tests factorial of 0.
TEST(FactorialTest, HandlesZeroInput) {
    // test instantiation of stubs and mocks
    auto &ble = ble::BLE::Instance();
    ble.setGattServer(nullptr);
    GattServerMock mock;

  EXPECT_EQ(Factorial(0), 1);
}


// Tests factorial of positive numbers.
TEST(FactorialTest, HandlesPositiveInput) {
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(2), 2);
  EXPECT_EQ(Factorial(3), 6);
  EXPECT_EQ(Factorial(8), 40320);
}


