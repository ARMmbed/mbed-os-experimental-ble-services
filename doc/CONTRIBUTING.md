# Contributing to Mbed OS Experimental BLE Services

Greetings!
Thank you for expressing an interest in Mbed OS Experimental BLE Services, an incubator for Bluetooth services.
This is a community-led effort so we welcome all contributions with open arms.

## How to contribute
We would like to make the contribution process as smooth as possible.
Therefore, we provide an out of the box development environment that can be used with Visual Studio Code.
Please refer to the main [development guide](./DEVELOPMENT.md) to learn how to setup and use this feature. 

Of course, you are free to use your own development environment. 
However, please ensure that all dependencies are present in the tree by running the command `bootstrap.sh` in the root repository.

## Types of contributions
There are a several ways to contribute to this repository, such as
* Opening a [new pull request](https://github.com/ARMmbed/mbed-os-experimental-ble-services/pulls) to add a service 
* Opening a [new issue](https://github.com/ARMmbed/mbed-os-experimental-ble-services/issues) to report a bug, request a feature or propose a service
* Opening a [new discussion](https://github.com/ARMmbed/mbed-os-experimental-ble-services/discussions) to ask a question, share an idea or simply engage with other community members   

## Testing
Pull requests that add new services should include unit and integration tests.

For unit tests, we use the [GoogleTest](https://github.com/google/googletest) framework with the [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) runner.
Please refer to the [unit testing guide](../tests/README.md) to learn how to write and run unit tests.

For integration tests, we use [pytest](https://docs.pytest.org/en/stable/) with [bleak](https://bleak.readthedocs.io/en/latest/).
Instructions on how to write and run integration tests can be found in the [integration testing guide](../tests/TESTS/README.md).

## Coding standard
We conform to the same [coding standard](https://os.mbed.com/docs/mbed-os/v6.7/contributing/style.html) as Mbed OS.
However, there are some exceptions to this, e.g. in BLE, where methods are `camelCase` instead of `snake_case`.

## Licensing and code ownership
This project is provided under an Apache-2.0 license.
As such, contributions to the project are accepted under the same license.

Please be aware that we expect service creators to maintain their services.
If you do not wish to take on maintenance responsibilities but would still like to see a service added to the repository, we recommend opening a service proposal.    
