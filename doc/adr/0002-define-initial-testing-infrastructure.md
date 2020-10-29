# 2. Define initial testing infrastructure

Date: 2020-10-28

## Status

Proposed

## Context

To raise the quality of the example maintained in this repository, contributors 
need a testing infrastructure available to validate new services proposed, 
extensions or bug fixes. 

### Unit testing 

Unit tests are isolated from the system and should run on desktop machines 
(Linux, MacOS, Windows). 
The unit tests will rely on mock and stub of BLE API, the test framework selected 
must support declaration of these kind of objects. 

Many C++ unit test framework are available (CppUTest, Catch2, Boost.test, 
google-test, ...) all with their qualities and weaknesses. 
Due to the use of google test in [Mbed OS](https://github.com/ARMmbed/mbed-os/tree/master/UNITTESTS) 
we propose to use google-test to unit test Bluetooth services.

### CI integration 

To validate new submissions, it is important to exercise them automatically. 
Many CI providers exists for open source project (Circle CI, travis, ...). 
We propose to use Circle CI as it is used to compile ble-example and ble-testsuite
submissions. It is easy to setup and approved by the ARMmbed organisation 

### Compilers 

GCC and Clang are both available to all machines targeted. 
We propose to use Clang as it provides better messages in case of compilation failure.

### Code coverage

To ensure tests submitted cover sufficciently the code submitted we propose to use the 
code coverage solution offered by the selected compiler (Clang).   

### Integration tests

There is no definitive pattern for integration test of Bluetooth services. 
This should be described and defined in a subsequent architecture decision when consensus has emerged.  


## Decision

We will use `google test` to unit test services proposed. The tests will be compiled 
with `Clang` and will generate a code coverage report when run.
For pull request validation, the code and test suite will be compiled and run by `Circle CI`. 
Depending on results and code coverage, the submission will be rejected. 

## Consequences

The base of the test infrastructure should be created, it must be based on google test, 
be compiled with clang run in Circle CI. 
