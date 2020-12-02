# 3. Define directory structure

Date: 2020-12-01

## Status

Accepted

## Context

The will be many services contributed by multiple people. We need a common way of structuring the services
so it's easy for the user to pick up any one service.

## Decision

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

## Consequences

All PRs amended to follow the structure. All future PRs comply with the structure. 
