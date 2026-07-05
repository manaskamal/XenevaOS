# Welcome to XenevaOS Documentation

Welcome to the official documentation for **XenevaOS**, a modern operating system built from scratch. This documentation portal provides all the resources needed to understand the OS architecture, compile the source code, set up virtual machines, and contribute to both kernel and user-space development.

Use the Table of Contents below to navigate through the project's documentation.

---

## 📖 Getting Started
- [**Introduction**](Introduction.md) — An overview of XenevaOS, its architecture, and its primary goals.
- [**Contributing Guidelines**](Contributing.md) — Rules and procedures for contributing to the repository.
- [**Preprocessor Macros**](Development/PreprocessorMacros.md) — Important compiler flags and definitions used across the codebase.

## 🛠️ Build & Setup
- [**Build Instructions (Windows/MSVC)**](BuildInstructions.md) — How to build XenevaOS on Windows using Visual Studio.
- [**Build Instructions (Linux/GCC)**](BuildInstructions(Linux).md) — How to compile the OS on Linux environments.
- [**Build Instructions (ARM64)**](BuildInstructions(ARM64).md) — Specific instructions for cross-compiling the ARM64 port.
- [**Populating Config**](PopulatingConfig.md) — Setting up the bootloader and kernel configuration files.
- [**VM Setup**](VMSetup.md) — Guide to configuring VirtualBox, VMWare, or QEMU manually.
- [**QEMU Automation**](QemuAutomation.md) — Using scripts for automated boot and testing on QEMU.
- [**Serial Terminal Attach**](SerialTerminalAttach.md) — How to attach to UART for low-level kernel debugging.

## 🧠 Kernel Internals
- [**About Kernel**](Kernel/AboutKernel.md) — Deep dive into the core architecture of the XenevaOS kernel.
- [**Boot Process**](Kernel/BootProcess.md) — Explanation of the hardware boot flow from UEFI to Kernel main.
- [**Memory Management**](Kernel/MemoryMangement.md) — Details on Physical, Virtual, and Heap memory allocation.
- [**Kernel Services (SCI)**](Kernel/KernelServices.md) — Overview of the System Call Interface (SCI) for user-space bridging.
- [**Virtual File System (VFS)**](Kernel/VirtualFileSystem.md) — Abstraction layer for file access, FAT32, and initrd parsing.
- [**Threads & Processes**](Kernel/Thread.md) — Internal workings of kernel threads and user processes.
- [**Drivers Subsystem**](Kernel/Drivers.md) — Core principles of the XenevaOS hardware driver model.
- [**USB Stack**](Kernel/usb.md) — How USB buses and controllers are initialized.

## 🚀 Development & User-Space
- [**XELoader (Dynamic Linker)**](XELoader.md) — The XenevaOS user-space PE linker and dynamic loading mechanism.
- [**Application Development**](Development/ApplicationDevelopment.md) — Writing user-space applications (GUI, Terminal) for XenevaOS.
- [**Driver Development**](Development/DriverDevelopment.md) — Developing dynamic drivers and interacting with kernel APIs.
- [**Project Architecture(MSVC)**](Development/ProjectArchitecture(MSVC).md)- Discusses various artifacts and project inter dependencies.
