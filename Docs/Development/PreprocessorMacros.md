## PREPROCESSOR MACROS for Kernel
 The Kernel project is divided into three category, __*Kernel,KernelAA64 and KernelRISCV64*__, and it's heavily depended on defined preprocessor macros to enable or disable specific codes within the kernel during build time. For example if kernel is targeted for specific hardware SoC, it can be build for that SoC just by enabling a specific macro or removing a specific macro. Same type of macros cannot be defined twice or else, kernel will behave unexpectedly.

 ### `__TARGET_BOARD_X__` defnitions
 Supported target boards are, please note that only one board can be defined at once during build time, defining multiple board difinition will cause the kernel to behave unexpectedly:
 - `__TARGET_BOARD_QEMU_VIRT__` : Builds the kernel for qemu virt board
 - `__TARGET_BOARD_RPI3__` : Builds the kernel for Raspberry Pi 3b plus
 - `__TARGET_BOARD_IMX8MP_VERDIN_DAHLIA__` : Builds the kernel targeting Verdin Dahlia board with nxp.imx8mp SoC
 - `__TARGET_BOARD_IMX8MP_SOC__` : Builds the kernel targeting nxp.imx8mp SoC <br><br>

 (_More board supports are work in progress_)

 ## `_USE_XALLOC` difinitions
 Xeneva supports dlmalloc and liballoc for kernel heap memories, once at a time can be defined during build time
 - `_USE_DLMALLOC` - will use Doug Lea's memory allocator for kernel heap memory
 - `_USE_LIBALLOC` - will use liballoc memory allocator for kernel heap memory

## `ARCH_X` difinitions
Arch specific definitions are mostly used in header files, which are shared among x86_64, arm64 and riscv64
- `ARCH_X64` - will force the project to use x86_64 targeted code
- `ARCH_ARM64` -- will force the project to use arm64 targeted code <br>

## KERNEL PROFILER definitions
Kernel uses inbuilt profiler to measure the latency values while performing certain tasks. Profiling heavily uses serial output to print profiling results. Profiler can be enabled or disabled as per need. 
- `__KERNEL_PROFILER_ON__` -- will tell the kernel to use profiler, without this kernel will bypass profiling outputs.

## Default Kernel expected macros
- `ARCH_ARM64` -- for arm64 build
- `__TARGET_BOARD_QEMU_VIRT__` - can be modified as per need.
- `_USE_DLMALLOC` - Uses Doug Lea's memory allocator
- `__STDC_LIMIT_MACROS` -- Default kernel expected
