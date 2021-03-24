# Testing experimental BLE services

This repository contains tools for testing experimental BLE services.

Unit tests, performed using the [GoogleTest](https://github.com/google/googletest) framework and the [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) runner, are located under [UNITTESTS](./UNITTESTS).

Integration tests, facilitated by [pytest](https://docs.pytest.org/en/stable/) and [bleak](https://bleak.readthedocs.io/en/latest/), can be found under [TESTS](./TESTS).

All services should be covered by both unit and integration tests. 
The [Link Loss Service (LLS)](../services/LinkLoss) is a good example of a fully tested service. 
