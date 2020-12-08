# 3. Define directory structure

Date: 2020-12-01

## Status

Accepted

## Context

There will be many services contributed by multiple people. We need a common way of structuring the services
so it's easy for the user to pick up any one service.

Some services might want to share some common code. We want to have that code structured so that include path shouldn't
change if these extensions are included into Mbed OS.

## Decision

### Services

Each service will be its own library. All services will be in the services directory.
Each service will contain at least an include folder and a mbed_lib.json.

An example service called `example` would look like this:

```
services/example/include/ble-service-example/example.h
services/example/source/example.cpp
services/example/mbed_lib.json
```

and the mbed_lib.json would contain at least:

```
{
    "name": "ble-service-example"
}
```

This way a user can clone the repo and include an individual service without getting unwanted includes.

### Extensions

If a service needs to share some common code between multiple services this code can be made into an extension.

Extensions are common code, extending the mbed-os ble feature. They are libraries of their own and each service that
wants to use them needs a `requires` section that pulls it in as a dependency.

And `example` extension library:

```
extensions/example/include/ble/gatt
extensions/example/include/ble/gap
extensions/example/include/ble/common
extensions/example/source/...
extensions/example/mbed_lib.json
```

and the extension mbed_lib.json would contain at least:

```
{
    "name": "ble-extension-example"
}
```

and the service needs it's mbed_lib.json expanded with a

```
    requires: ["ble-extension-example"]
``` 

### Tests

Unit tests and integration tests go into the tests directory:

```
tests/UNITTESTS
tests/TESTS
```

`TESTS` and `UNITTESTS` are automatically ignored by mbed tools when not building tests. Any other files or directories in
the tests folder need to be added to `.mbedignore` in order to avoid compilation when the repo is included with the
intention to use one of the services.

## Consequences

All PRs amended to follow the structure. All future PRs comply with the structure. 
