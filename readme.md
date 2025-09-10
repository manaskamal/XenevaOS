
[![Discord](https://img.shields.io/discord/1255326572617924618?label=Discord&logo=discord&style=flat)](https://discord.com/invite/YNsY7hhQ)


![alt text](https://github.com/manaskamal/XenevaOS/blob/master/XeLogo.png?raw=true)

XenevaOS is an operating system built from scratch targeted at modern hardware. The OS is designed for both x86 and ARM architecture with plans for RISC-V  implementation in the near future as well. Our OS is built on our very own hybrid kernel known as _'Aurora'_.<br> Please visit [Build Instructions](Docs/BuildInstructions.md) to build the project

## Documentation
Refer to [Documentations of XenevaOS](Docs/Introduction.md)

## Contributing to the project
XenevaOS is an open-source project that welcomes contributions from developers, researchers, and enthusiasts who are passionate about low level system development. Whether you're intereseted in Kernel development, driver development, low level graphics or application level features there are many ways to get involved. We encourage contributions in the form of code, documentation, bug reports, and feature suggestions. If you're interested in contributing, check out out [Contribution Guideline](Docs/Contributing.md), explore open issues, and join the discussion to help shape the project.

## __Features__ :
- ACPI (through acpica)
- x64 and arm64 Kernel
- Fully Higher Half Memory with MMIO mappings
- Driver _loading and linking_ through dll files
- SSE/SSE3
- USB3
- Intel High Definition Audio
- Multiprocessor support (muliprocessor scheduler not ready !!)
- Application loader and dynamic library (shared library)
- Freetype2 as font rendering engine
- Compositing Window Manager called "Deodhai"
- Graphics Library known as "Chitralekha"
- Xeneva Terminal with basic ANSI/VT100 escape sequence support
- Desktop environment called Namdapha Desktop
- Storage supports : AHCI/SATA, NVMe
- USB3 Class drivers (USB HID Mouse, USB Tablet, USB MSC driver) ..etc
- Audio Server called "Deodhai-Audio" with 
    (44kHz/16bit audio format, stereo/mono panning, gain control)
- Networking (IPv4, UDP/IP, TCP/IP, ICMP)
- Upcoming RISC-V implementation

_And many more coming soon_

## Screenshot (x86_64) :

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/XenevaV1_1.png?raw=true)

## AppTray (x86_64 ) :

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/apptray.png?raw=true)

## Homescreen (ARM64) :

<img width="2187" height="1179" alt="Screenshot (1867)" src="https://github.com/user-attachments/assets/60a9ce72-14c8-44ea-98da-5f03d27583a9" />


## App Tray (ARM64) :

<img width="2560" height="1440" alt="Screenshot (1889)" src="https://github.com/user-attachments/assets/a2cb77c3-b004-4e81-9d97-31c341c22e3d" />



# Installed Applications :

- Audio player called Accent Player
- File Browser
- Calculator
- Xeneva Terminal (ANSI/VT100 support)
- XEShell
- Audio Server called (DeodhaiAudio)
- play (_CLI Application_)
- piano (_CLI Application_)
- Calendar 
- Background Services like - Net Manager


# Building the project

The project is build purely on Windows Environment. Please Visit [Build Instructions page ](Docs/BuildInstructions.md) for more information

# Purpose and Goals
While writing an Operating System from scratch is a very fun and knowledgeable journey, at the same time it is a deeply challenging one. The project was not started with a specific goal or purpose in mind; rather, it began as a passionate solo developer project that aims to learn and address what existing Operating Systems lack. As the project grew over time, it became necessary to have a clear purpose and goal. Even though it once was a solo development project, XenevaOS has now evolved from a one-person effort to a collaborative team with a shared interest. The strategies and execution of the project are planned and carried out carefully with a deliberate intention.


## The Purpose:
While there are many mature Kernels available for different domains and they are already feature-complete, they also come with layers of legacy code that make it difficult for developers to implement or experiment with new ideas on top of them. XenevaOS is completely built from scratch and aims to provide a flexible playground for experiments with new designs and architectures with modern computing in mind without compromising on software implementation standards and performance. 


## The Goals:
- XenevaOS aims to target modern hardware with modern computing architecture in mind. 
- To experiment and build an OS from ground the ground up transitioning away from traditional OS design as much as possible. 
- To ensure that the project has minimal software abstractions as it grows over time in the future for better performance without affecting software implementation standards.
- To make the software as robust as possible, to handle software dependencies and errors transparently without affecting the user's flow.
- To research on implementing some new low level architecture with security concerns in mind like lightweight sandboxes and memory safety technique.
- XenevaOS aims to be a single core with multiple domain. For example (_XR(AR/VR), Robotics, Custom Niche Solutions etc_.).
- XenevaOS aims to have native 3D interface as GUI making it suitable for _XR(AR/VR)_ computing. 

**NOTE: XenevaOS is in early development. Some of the features above are not implemented yet, because the project is in foundation level. But we are committed to exploring each of them through focused iterations and community feedback. Whether you're a developer looking to contribute, or someone exploring alternative OS design, we welcome your curiosity.**


# Contact:
For questions, suggestions, or collaborations, feel free to reach out
- Email: hi@getxeneva.com
- Website: [www.getxeneva.com](https://getxeneva.com)

# License:
XenevaOS is licensed under the [BSD 2-clause License](./LICENSE)
