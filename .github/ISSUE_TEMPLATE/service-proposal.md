---
name: Service proposal
about: Template to use to submit a service proposal
title: "[SERVICE]"
labels: service
assignees: ''

---

<!-- 

Use the following template when submitting proposals for new CUSTOM services to this repository.

 Linking to a standard characteristic/service specification document on the Bluetooth SIG's website is sufficient for standard characteristic/service proposals. When proposing standard services, also include specific requirement information for the Mbed BLE implementation of this service (eg: memory constraints, baremetal support, etc) 

The following site can be used to generate a long-format UUID for custom services/characteristics: https://www.uuidgenerator.net/
This site is useful for generating markdown-formatted tables: https://www.tablesgenerator.com/markdown_tables
-->

### BLE Standard Service: Yes/No
### Proposed UUID(s) or link to specification document:

| Svc Name                 | Char Name         | UUID   | Qualifier | Mandatory Props | Optional Props | Security |
|--------------------------|-------------------|--------|-----------|-----------------|----------------|----------|
| ServiceName | -                 | AssociatedUUID | -         | -               | -              | -        |
| -                        | CharacteristicName | AssociatedUUID | O         | Read            |                | None     |

## Overview:
Give overview of what the proposed services are intended to do.

## Detailed Description
Describe each service/characteristic in more detail for each include the following information: brief description of purpose for characteristic, functional requirements, non-functional requirements (eg: build w/o RTOS, memory size constraints, etc), ideas for API.

### Characteristic Description
Each characteristic should be described individually. 

**Characteristic Behavior**: Describe the behavior associated with the characteristic access (Data exchange, format, error code, side effects, ...). 

**Functional requirements:** e.g: Must be compatible with baremetal builds
