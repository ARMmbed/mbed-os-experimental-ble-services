# plantuml
This directory contains UML sequence diagrams for the Link Loss Service (LLS). 
The [PlantUML](https://plantuml.com/) files are located in the `source` folder.
These can be copied into the [Online Server](http://www.plantuml.com/plantuml) or opened in any IDE that supports PlantUML.
The diagrams can be located in PNG format under the `img` folder, but are embedded in this README for ease of viewing.

## Purpose
The purpose is to illustrate the main sequences present in an application that uses a Link Loss Service, such as the [BLE_GattServer_ExperimentalServices](https://github.com/ARMmbed/mbed-os-example-ble/tree/master/BLE_GattServer_ExperimentalServices) example.

Sequence diagrams can aid in the development of unit tests. 
The tests in `tests/UNITTESTS/LinkLoss` are a reflection of the sequence diagrams provided here. 
It is recommended to create sequence diagrams before writing unit tests for new services.

## Sequence Diagrams

![normal_initialization.png](img/normal_initialization.png)

**Fig. 1. Normal Initialization**

![normal_connection.png](img/normal_connection.png)

**Fig. 2. Normal Connection**

![normal_disconnection.png](img/normal_disconnection.png)

**Fig. 3. Normal Disconnection**

![normal_write_request.png](img/normal_write_request.png)

**Fig. 4. Normal Write Request**

![invalid_write_request.png](img/invalid_write_request.png)

**Fig. 5. Invalid Write Request**

![disconnection_reconnection.png](img/disconnection_reconnection.png)

**Fig. 6. Disconnection Reconnection**

