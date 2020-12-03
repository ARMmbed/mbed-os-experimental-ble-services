# Experimental services for Mbed OS BLE

This repository is an incubator for standard and common Bluetooth services development.

## Contents

The repo contains multiple BLE services designed to work with Mbed OS. They are not officially supported and are not
part of Mbed OS itself. This is a community led effort and we welcome all contributions, which should follow the
decisions published in the `doc/adr` directory.

Services are are in the `services` directory. Each BLE service is an Mbed OS library. The name of each library
is in the `mbed_lib.json` file inside each of the service directories. They follow the pattern `ble-service-name`. 

## How to use a service

Inside your Mbed OS project, include a file called 'mbed-os-experimental-ble-services.lib' with contents:

```
https://github.com/ARMmbed/mbed-os-experimental-ble-services
```

Call `mbed deploy`. This will check out this repository into your project.

To use a particular service you will need to now add the library that contains it into your application.

Inside your `mbed_app.json` you need to override the `requires` section for your platform.
Add the name of the library containing the service.

For example, if you want to use the DFU service, add this to your `mbed_app.json`: 

```
    "target_overrides": {
        "*": {
            "target.requires": [ "ble-service-DFU" ]
        },
    }
```

### License and contributions

The software is provided under Apache-2.0 license. Contributions to this project are accepted under the same license.
