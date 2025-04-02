# XenevaOS documentation

## Introduction
Welcome to the XenevaOS documentation. XenevaOS is an operating system completely written from scratch with a focus on performance and modern hardware support while providing a robust modern graphical environment for daily needs of user. [Let's get started](#welcome-to-xenevaos--empowering-creativity)

## NOTE
The project is still under development and not fully matured. The project requires lot of contribution and developments to various aspects of the system.

## _What is XenevaOS?_
The term _XenevaOS_ made up of four subsystems : (_The Kernel_, _Device Drivers_, _Services_, _Applications_). In easy language we can say, _'XenevaOS' is a collection of various components, where each components works individually. _The Kernel_ being the important component of all, where device drivers, services and applications depends. Every components are binded up by _'Service call layer'_ (which is _'system calls'_ in general language).

## Core Objectives of XenevaOS
- __Modern Hardware Support__: XenevaOS aims to support modern hardware. Which help and will help the system to follow up the technology trends. It aims to be engineered to take the full advantage of the latest advancements in hardware, ensuring a good performance and stability.
- __Enhanced Graphics and Audio__ : Xeneva OS aims to provide good stack of graphics and audio with supporting modern specifications of Graphics and Audio Hardware. The project aims to provide a good stack to audio and video production centric choices.
- __Simple User Interface__ :  _'The function of good software is to make the complex appear to be simple'_ -_Grady Booch_. The Project aims to provide simple user friendly graphical interface while targeting modern design choices. 

## Getting Started
__Please NOTE that, though the documentation mentioned _'Core Objectives of XenevaOS'_ and some goals of the project. But still the system lacks various functionalities and doesn't seems to go through the mentioned goals. Still the project is under development and can promise that the system will come with all the mentioned goal. Just needs your support and contributions to make it happen.__<br>
This documentation will guide you through all the parts of XenevaOS and how it works. Whether you are a developer looking to contribute to XenevaOS or a user seeking to understand its inner workings, this documentation aims to provide comprehensive and accessible information.

## Welcome to XenevaOS -_Empowering Creativity_

__Build the project__:
- [Build instructions](BuildInstructions.md)
- [Running XenevaOS](VMSetup.md)

__Kernel__ : 
- [About the Kernel](Kernel/AboutKernel.md)
- [Boot Process](Kernel/BootProcess.md)
- [Memory Management](Kernel/MemoryMangement.md)
- [Drivers](Kernel/Drivers.md)
- [USB](Kernel/usb.md)

__Components__ :
- [XELoader](XELoader.md)

__Development__ :
- [Writing Drivers for XenevaOS](Development/DriverDevelopment.md)
- [Writing User Applications for XenevaOS](Development/ApplicationDevelopment.md)
 