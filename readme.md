# Xeneva

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/XeLogo.jpg?raw=true)

Xeneva is a GUI based operating system for 64 bit architecture based processors. For x86_64
bit systems, it can be booted from UEFI environment. The Kernel is known as _'Aurora'_ 
with hybrid design and the entire operating system is known as _'Xeneva'_.

## __Features__ :
1. ACPI (through acpica)
2. x64 based Kernel
3. Fully Higher Half Memory with MMIO mappings
4. Driver _loading and linking_ through dll files
5. SSE/SSE3
6. USB3
7. Intel High Definition Audio
8. Multiprocessor support (muliprocessor scheduler not ready !!)
9. Application loader and dynamic library (shared library)
10. Freetype2 as font rendering engine
12. Compositing Window Manager called "Deodhai"
13. Graphics Library known as "Chitralekha"
13. Xeneva Terminal with basic ANSI/VT100 escape sequence support
14. Desktop environment called Namdapha Desktop
15. Storage supports : AHCI/SATA, NVMe
16. USB3 Class drivers (USB HID Mouse, USB Tablet) ..etc
17. Audio Server called "Deodhai-Audio" with 
    (44kHz/16bit audio format, stereo/mono panning, gain control)


_And many more coming soon_

## Screenshot:

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/XenevaV1_2.png?raw=true)

## AppTray:

![alt text](https://github.com/manaskamal/XenevaOS/blob/master/apptray.png?raw=true)

# Installed Applications :

1. Basic GUI Wave Audio player called Accent Player
2. Basic File Navigator App
3. A Calculator
4. Xeneva Terminal (ANSI/VT100 support)
5. Basic shell application called XEShell
6  Terminal Application : play(wave player)

# Building the project

The project is build on Windows environment. It requires Microsoft 
Visual Studio 2013 or later.
By downloading the project, once can open the solution file (_.sln_) which
s configured as per the project requirements.
_NOTE: The project directories are need to be manually configured for now_
NASM is used as the main assembler. Setup of nasm under Visual Studio is
also required.

## __Requirements__ :
1> Nasm as the main assembler <br>
2> Microsoft Visual Studio 2013 or later <br>
3> GPT configured FAT32 partition ( EFI-System Partition ) to store 
   the Kernel {_'xnkrnl.exe'_} and the bootloader {_'BOOTx64.efi'_}.

# Contact:

Email: manaskamal.kandupur@gmail.com