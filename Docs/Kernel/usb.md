# USB System in Xeneva

The USB system in Xeneva is designed with modularity and flexibility in mind, which allows the communication between higher-level applications with USB devices and the Kernel with USB devices. The system is composed of three different layer running separately from each other: the __Host Controller Driver__, the __Core USB Driver__ and the __USB Class Drivers__. Each components are loaded separately by the kernel as module to ensure clean separation and ease of development.

## Architecture Overview

- **Host Controller Driver** : Host Controller Driver is responsible for directly interacting with the hardware. It handles tasks such as initializing the controller itself, managing port attach and dettach events, managing endpoint descriptors and transferring data to and from the USB devices to USB class drivers. This driver is separate from the USB core driver to maintain a clean abstraction layer.

- **USB Core Driver** : The USB core driver acts as a glue layer between the USB class drivers and the host controller driver. It manages device enumeration, commnication protocols, and standard USB operations. 
