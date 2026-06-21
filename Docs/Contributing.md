# Contributing to XenevaOS

Thank you for your interest in contributing to XenevaOS!

XenevaOS is a modern, lightweight operating system that focuses on performance, multimedia capabilities, and synchronization with current technologies. This guide outlines the contribution process and best practices to help maintain the quality and stability of the project.
## Getting Started
XenevaOS uses Microsoft Visual Studio as a development platform and requires a Windows environment. [Click here for Build Instructions](BuildInstructions.md), which highlights all the required steps to set up the environment.

## Types of Contributions

### *Kernel Development*
- XenevaOS's kernel _'Aurora'_ is written purely in C and assembly. The kernel is the main component and a highly privileged layer of the operating system. Any incorrect code or untested changes can lead to system-wide crashes, instability, or security vulnerabilities.

- Always test kernel modifications in a controlled environment like QEMU or VirtualBox before pushing changes.

- Follow memory management, system calls and other subsystem guidelines as outlined in the documentation.

- Before making major changes, open an issue or discussion in the repository or on Discord to align with the project's direction and avoid duplicate work.

### *Coding Style*
The XenevaOS project follows a consistent coding style to maintain readability and discipline. All contributions should adhere to this style to ensure uniformity across the codebase. Well-structured, clean, and documented code is highly valued, as it improves maintainability and collaboration. 

- Use _'PascalCase'_ for function naming and Structs. 
- If your contribution is for the Kernel, use PascalCase for function naming with the prefix __'Au'__, for example: ```AuPmmngrAlloc```
- Use _'snake_case'_ for variable naming 
- Use __*lowercase*__ for file names and __*PascalCase*__ for folder names.
- Keep functions modular and documented with comments.

### *Bug Reports & Feature Requests*
- Open an issue to report bugs or suggest new features.
- When reporting a bug, provide clear details about the issue, including system debug messages from the serial terminal, the processor state, and register values. If possible, describe the conditions under which the bug occurs.
- Mention your testing environment.
- For feature requests, explain the purpose and potential benefits to the project.

### *Conclusion*
Thank you for your interest in contributing to XenevaOS! Your contributions — whether in code, documentation, bug reports, or discussions — help shape the future of this project. If you have any questions or need guidance, feel free to open an issue or join the discussions on GitHub or Discord. Let's work together to make a powerful and efficient operating system!

