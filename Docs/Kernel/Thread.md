# Threading in XenevaOS

In XenevaOS, multitasking is entirely built on the concept of threads. XenevaOS does not identify processes or user applications directly; instead, it only schedules and executes individual threads using CPU resources. In simple terms, a thread is the smallest unit of execution in XenevaOS. The scheduler is the heart of XenevaOS and is responsible for managing threads, handling creation, blocking/unblocking (synchronization), context switching, and terminating threads when required.

Each thread is defined by a structure that stores its current CPU registers/state, stack pointer, memory information, and scheduling state.

_coming soon_...