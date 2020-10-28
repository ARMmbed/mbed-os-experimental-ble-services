---
name: Service proposal
about: Template to use to submit a service proposal
title: "[SERVICE]"
labels: service
assignees: ''

---

<!-- 

Use the following template when submitting proposals for new services to this repository.

If you are proposing a characteristic/service that already has a detailed specification document available, it is sufficient to attach a PDF of that document (preferred, as allowed by document license) and/or provide a link to where the document is available for download. However, you must still provide a brief overview of the service, why you think it should be added to this repository, as well as any specific implementation requirements (eg: memory constraints, baremetal support, etc).

The following site can be used to generate a long-format UUID for custom services/characteristics: https://www.uuidgenerator.net/
This site is useful for generating markdown-formatted tables: https://www.tablesgenerator.com/markdown_tables

The Bluetooth SIG service specifications (which can be found here: https://www.bluetooth.com/specifications/gatt/) provide excellent examples of what information should be included when proposing a service. Some things to consider for each service/characteristic are: is the characteristic mandatory or optional? What kind of properties (eg: read, write, notify, indicate, etc) are mandatory? Which are optional? Are there any security requirements for the service/characteristic?

-->

### BLE Standard Service:

- [ ] Yes
- [ ] No

### Proposed UUID(s) or link to specification document:

**Service Name:**
**UUID:**

**Characteristic Name:**
**UUID:**
**Mandatory?**
- [ ] Yes
- [ ] No
**Mandatory Properties:**
**Optional Properties:**
**Security Requirements:**

## Overview:
Give overview of what the proposed services/characteristics are intended to do.

## Detailed Description
Describe each service/characteristic in more detail for each include the following information: brief description of purpose for characteristic, functional requirements, non-functional requirements (eg: build w/o RTOS, memory size constraints, etc), ideas for API.

### Characteristic Description
Each characteristic should be described individually. 

**Characteristic Behavior**: Describe the behavior associated with the characteristic access (Data exchange, format, error code, side effects, ...). 

**Functional requirements:** e.g: Must be compatible with baremetal builds
