# XenevaOS documentation

## Introduction
Welcome to the XenevaOS documentation. XenevaOS is an operating system completely written from scratch with a focus on high performance and modern hardware support. It aims to provide a robust graphical environment for everyday use, and its lightweight, secure, and adaptable design makes it ideal for both developers and general users. [Let's get started](#welcome-to-xenevaos--empowering-creativity)

## NOTE
The project is still under development and not yet fully mature. It requires significant contributions and development to various aspects of the system.

## _What is XenevaOS?_
The _XenevaOS_ architecture is composed of four subsystems : (_The Kernel_, _Device Drivers_, _Services_, _Applications_). Each component works individually, with _The Kernel_ being the most crucial as the other components like _Device Drivers_, _Services_ & _Applications_ depend on it. All components are bound together by the _'Service call layer'_ (commonly known as _'system calls'_).

## Core Objectives of XenevaOS
- __Modern Hardware Support__: XenevaOS is engineered to utilize all the latest advancements of modern hardware. The project aims to provide optimal performance and stability while keeping up with technology trends.
- __Enhanced Graphics and Audio__ : XenevaOS aims to provide good stack of graphics and audio by supporting modern specifications of Graphics and Audio Hardware. The project aims to be an ideal choice for audio and video production.
- __Simple User Interface__ :  _'The function of good software is to make the complex appear to be simple'_ `_Grady Booch_. XenevaOS aims to provide a simple, user-friendly graphical interface that incorporates modern design principles.
- __Minimal Abstraction__ : XenevaOS aims to have minimal software abstractions, whichs helps enhance performance.
- __User-First Priority__ : XenevaOS aims to prioritizes the users and their experienece by reducing software dependencies.

## Getting Started
__NOTE:__ While this documentation outlines the core objectives and goals of the project, XenevaOS is still under active development and currently lacks many of these functionalities. We promise that the system will eventually achieve these goals with your support and contributions.<br> 

This documentation will guide you through all aspects of XenevaOS and how it works. Whether you are a developer looking to contribute to XenevaOS or a user seeking to understand its inner workings, this guide aims to provide comprehensive and accessible information.

## Welcome to XenevaOS -
__Build the project__:
- [Build instructions](BuildInstructions.md)
- [Running XenevaOS](VMSetup.md)

__Kernel__ : 
- [About the Kernel](Kernel/AboutKernel.md)
- [Xeneva Kernel Boot Protocol](Kernel/AboutKernel.md#xeneva-kernel-boot-protocol)
- [Boot Process](Kernel/BootProcess.md)
- [Memory Management](Kernel/MemoryMangement.md)
- [Threading in Xeneva](Kernel/Thread.md)
- [Drivers](Kernel/Drivers.md)
- [USB](Kernel/usb.md)

__Components__ :
- [XELoader](XELoader.md)

__Development__ :
- [Writing Drivers for XenevaOS](Development/DriverDevelopment.md)
- [Writing User Applications for XenevaOS](Development/ApplicationDevelopment.md)
 
