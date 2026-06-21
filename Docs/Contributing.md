# Contributing to XenevaOS

Thank you for your interest in contributing to XenevaOS!

XenevaOS is a modern, lightweight operating system that focuses on performance, multimedia capabilities, and synchronization with current technologies. This guide outlines the contribution process and best practices to help maintain the quality and stability of the project.
## Getting Started
XenevaOS historically used Microsoft Visual Studio as the primary development platform on Windows. However, the project is actively being ported to Linux using the GCC toolchain. 
- For Windows/MSVC: [You can click here for Build Instructions (Windows)](BuildInstructions.md).
- For Linux/GCC: [You can click here for Build Instructions (Linux)](BuildInstructions(Linux).md).

## Types of Contributions

### *Kernel Development*
- XenevaOS's kernel _'Aurora'_ is purely written in C and some assembly. Kernel is the main component and highly priviledged layer of an operating system. Any incorrect code or untested changes can lead to system-wide crashes, instability, or security vulnerabilities.

- Always test kernel modificaions in a controlled environment like QEMU, VirtualBox before pushing changes.

- Follow memory management, system calls and other subsystem guidelines as outlined in the documentation.

- Before making major changes, open an issue or discussion in the repository or on Discord to align with the project's direction and avoid duplicate work.

### *Coding Style*
XenevaOS project follows good coding style to maintain consistency, readability, and project discipline. All contributions should adhere to this style to ensure uniformity across the codebase. Well-structured, clean, and documented code is highly valued, as it improves maintainability and collaboration. 

- Use _'PascalCase'_ for function naming and Structs. 
- If your contribution is for Kernel, use PascalCase for function naming with the prefix __'Au'__, For example - ```AuPmmngrAlloc```
- Use _'snake_case'_ or _'PascalCase'_ for variable naming.
- Use __*lowercase*__ for C/C++ source and header files, and __*PascalCase*__ for folder names.
- Use __*PascalCase*__ for documentation (`.md`) files.
- Keep functions modular and documented with comments

### *Bug Reports & Feature Requests*
- Open an issue to report bugs or suggest a new features
- When reporting a bug, provide clear details about the issue, including system debug messages from the serial terminal, processor state and register values. If possible, describe the conditions under which the bug occurs.
- Mention your testing environment
- For feature requests, explain the purpose and potential benefits to the project.

### *Conclusion*
Thank you for your interest in contributing to XenevaOS! Your contributions-whether in code, documentation, bug reports, or discussions-help shape the future of this project. If you have any questions or need guidance, feel free to open an issue or join the discussions on GitHub or Discord. Let's work together to make a powerful and efficient operating system !

