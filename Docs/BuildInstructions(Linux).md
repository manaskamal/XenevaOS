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

## Running in QEMU (Linux/ARM64)
To automate the creation of the FAT32 boot image and launch the OS in QEMU, use the provided bash script:
1. Navigate to the root of the XenevaOS repository.
2. **(Temporary Dev Workaround)** Place a pre-built `initrd2.img` (containing GUI/resources) at the root of the repository. *Note: Using a pre-built ramdisk is a temporary development workaround for the active porting phase to help Linux developers test the user-space GUI easily. In the future, this will be replaced by a dedicated native build step.*
3. Run the script:
   `./Scripts/Linux/build_and_run_qemu.sh`

*Note: The GCC build for ARM64 automatically defines `__TARGET_BOARD_QEMU_VIRT__`, which tells the bootloader to bypass the interactive screen resolution menu. This allows QEMU to boot directly into the OS without hanging for user input.*

### Building `initrd2.img` Manually
If you do not have a pre-built `initrd2.img` or want to generate a fresh one from the `Resources/resources/` directory, you can pass a flag to force a manual build:
`./Scripts/Linux/build_and_run_qemu.sh --force-manual-build`
*Note: This will require `mkfs.vfat` and `mcopy` (from the `mtools` package) to be installed on your Linux system.*
