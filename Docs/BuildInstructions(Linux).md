# XenevaOS Build Instructions (Linux / GCC)

## Prerequisites
To build XenevaOS natively on Linux without relying on MSVC, ensure you have the appropriate GCC toolchain installed.
- For x86_64 Kernel: `g++` and `make` (multilib support may be required).
- For ARM64 (KernelAA64): `aarch64-linux-gnu-gcc` and `aarch64-linux-gnu-binutils`.

## Building the ARM64 Kernel (KernelAA64)
1. Open a terminal and navigate to the `KernelAA64/` directory.
2. Run `make clean` to clear any old MSVC objects or stale binaries.
3. Run `make` to compile the `.c` and `.s` files.
4. The Makefile will automatically link the objects into `KernelAA64.elf` and generate a PE32+ binary (`KernelAA64.exe`) using `aarch64-linux-gnu-objcopy`.

## Building the x86_64 Kernel (Kernel)
1. Open a terminal and navigate to the `Kernel/` directory.
2. Run `make clean` and then `make` to compile the C++ source files into the higher-half kernel elf format.

*Note: The GCC port is actively being developed. More subsystems and drivers will be added to the GCC build pipeline over time, moving completely away from Windows-specific Visual Studio configurations.*
