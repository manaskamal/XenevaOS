# XenevaOS

[![Discord](https://img.shields.io/discord/830522505605283862.svg?logo=discord&logoColor=white&logoWidth=20&labelColor=7289DA&label=Discord&color=17cf48)](https://discord.com/invite/4Kj8x7v6h8)


![alt text](https://github.com/manaskamal/XenevaOS/blob/master/XeLogo.jpg?raw=true)

Xeneva is a GUI based operating system for 64 bit architecture based processors. For x86_64
bit systems, it can be booted from UEFI environment. The Kernel is known as _'Aurora'_ 
with hybrid design and the entire operating system is known as _'Xeneva'_.<br> Please visit [Build Instructions](Docs/BuildInstructions.md) to build the project

## Documentation
Refer to [Documentations of XenevaOS](Docs/Introduction.md)

## Contributing to the project
XenevaOS is an open-source project that welcomes contributions from developers, researchers, and enthusiasts who are passionate about low level system development. Whether you're intereseted in Kernel development, driver development, low level graphics or application level features there are many ways to get involved. We encourage contibutions in the form of code, documentation, bug reports, and feature suggestions. If you're interested in contibuting, check out out [Contribution Guideline](Docs/Contributing.md), explore open issues, and join the discussion to help shape the project.

## __Features__ :
- ACPI (through acpica)
- x64 based Kernel
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
- USB3 Class drivers (USB HID Mouse, USB Tablet) ..etc
- Audio Server called "Deodhai-Audio" with 
    (44kHz/16bit audio format, stereo/mono panning, gain control)
- Networking (IPv4, UDP/IP, TCP/IP, ICMP)


_And many more coming soon_

## Screenshot:

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/XenevaV1_1.png?raw=true)

## AppTray:

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/apptray.png?raw=true)

# Installed Applications :

- Audio player called Accent Player
- File Browser
- Calculator
- Xeneva Terminal (ANSI/VT100 support)
- XEShell
- Audio Server called (DeodhaiAudio)
- play (_CLI Application_)
- piano (_CLI Application_)
- Calender 
- Background Serivices like - Net Manager


# Building the project

The project is build purely on Windows Environment. Please Visit [Build Instructions page ](Docs/BuildInstructions.md) for more information


## __Requirements__ : 
1> NASM as the main assembler <br>
2> Microsoft Visual Studio 2013 or later <br>
3> GPT configured FAT32 partition ( EFI-System Partition ) to store 
   the Kernel {_'xnkrnl.exe'_} and the bootloader {_'BOOTx64.efi'_}.

# Contact:

Email: manaskamal.kandupur@gmail.com