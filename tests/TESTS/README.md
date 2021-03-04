# Integration testing

This repository contains a set of tools that can be used to write integration tests for Experimental BLE Services.
The approach taken is based on [bleak](https://bleak.readthedocs.io/en/latest/), a GATT client software capable of connecting to BLE devices acting as GATT servers.
As such, only a single host and board are required to develop and run the tests.

## Test code structure
Each test suite should be named after the service under test (SUT) and contain two folders: device and host.
The former is a BLE application containing the SUT, and should be built and flashed onto the board prior to running the tests.
The latter is home to one or more test scripts that test the SUT's system-level functionality.
For example, the directory tree for the LinkLoss test suite is shown below.

```
LinkLoss/
├─── device/
│    │─── source/
│    │    │─── DisconnectionService.cpp
│    │    │─── DisconnectionService.h
│    │    └─── main.cpp
│    │─── ~mbed-os
│    │─── ~LinkLoss
│    │─── CMakeLists.txt
│    └─── mbed_app.json
└─── host/
     │─── __init__.py
     └─── test_link_loss.py
```

The ~mbed-os and ~LinkLoss files are symbolic links for the Mbed OS clone in the parent tests directory, and the Link Loss Service (LLS) files in the services subdirectory of the root Experimental BLE Services repository, respectively.
These are added during the bootstrap process using the `symlink` command, a platform agnostic method to create symbolic links.
The general syntax is:

```
symlink <source> <destination>
```

## Integration testing with pytest
The [pytest](https://docs.pytest.org/en/stable/) framework is used to write the tests. 
However, bleak makes extensive use of [coroutine functions](https://docs.python.org/3/library/asyncio-task.html#id1), which are not natively supported by pytest.
To test asynchronous code, we need to modify the test runner so that it uses asyncio tasks to execute tests defined as coroutines.
Luckily, we can do this with the help of the `pytest.mark.asyncio` decorator included in the **pytest-asyncio** plugin for pytest. For example, the code below tests the initial value of the alert level in the Link Loss Service (LLS) by reading the alert level characteristic and comparing the result to the constant `NO_ALERT`. 
The parameters `board` and `client` are test fixtures and are discussed in detail in the next section.

```
@pytest.mark.asyncio
async def test_read_alert_level_initial_value(board, client):
    assert await client.read_gatt_char(UUID_ALERT_LEVEL_CHAR) == NO_ALERT
```

### Test fixtures
Each test should take at least two parameters: `board` and `client`.
These are test fixtures that allocate and release `SerialDevice` and `BleakClient` objects before and after each test.
The code block below shows their definitions in the case of the LinkLoss test suite.
The string passed to the allocate functions should be equal to the name of the device under test (DUT).

```
@pytest.fixture(scope="function")
def board(board_allocator: BoardAllocator):
    board = board_allocator.allocate('LinkLoss')
    yield board
    board_allocator.release(board)


@pytest.fixture(scope="function")
async def client(client_allocator: ClientAllocator):
    client = await client_allocator.allocate('LinkLoss')
    yield client
    await client_allocator.release(client)
```

### Building and running integration tests
1. Run the bootstrap process: `../bootstrap.sh`

1. Enter the device folder and compile and flash the BLE application onto the board:

   ```
   cd <service>/device
   mbed compile -t <toolchain> -m <target> -f 
   ```

   where, `<service>` is the name of the test suite, e.g. `LinkLoss`

1. Run the tests by passing the host folder to pytest:

   ```
   python -m pytest <service>/host
   ```

   On some platforms, it is required to specify the target and port in additional arguments:

   ```
   python -m pytest <service>/host --platforms=<target> --serial_port=<port>
   ```

   where, `<port>` is the name of serial port connected to the DUT, e.g. `COM8`