# 03_kernel_services.md - Kernel-Level API & System Services

This file outlines kernel-level capabilities, low-level APIs, and system service interfaces available to developers on XenevaOS. These features are useful for developers writing system apps, service daemons, or working with memory, processes, and I/O directly.

---

## 🧠 Memory Management

### Virtual Memory Model
- Higher-half kernel (mapped at 0xC0000000)
- 4KB pages with identity-mapped physical memory during early boot
- Paging enabled via CR3
- Plans for user memory sandboxing and permission bits

### Memory Allocation API (from XECLib)
```c
void* malloc(size_t size);
void free(void* ptr);
void* memset(void* dest, int val, size_t len);
void* memcpy(void* dest, const void* src, size_t len);
```

---

## 🔃 Process & Tasking System

### App Loading
- Apps compiled to `.xe` format (PE-like)
- Loaded using `XELoader`, which sets up memory, symbol tables, and entry point

### Scheduler
- Cooperative round-robin (current)
- Plans for: preemptive scheduling, priority classes, per-core CPU affinity

### Syscalls
- Accessed via software interrupt gate (INT 0x80-style)
- Wrapper functions exposed via XECLib

---

## 📨 Inter-Process Communication (PostBox)

### Message API
```c
void send_message(int pid, const char* message);
char* receive_message(int pid);
```

- Messages are copied via kernel ring buffers
- Planned: shared memory + file-backed buffers
- Can be used for app ↔ daemon ↔ system comms

---

## 📂 Filesystem Access

### Syscall Layer (Wrapped via XECLib)
```c
int file_open(const char* path);
int file_read(int fd, void* buffer, int size);
int file_write(int fd, const void* buffer, int size);
int file_close(int fd);
```

- Backed by VFS in progress (currently FAT/raw disk reads)
- File descriptors allocated per-process

---

## 🖥 Display System Interface

### Deodhai Compositor
- All windows are registered with the compositor
- Buffer updates are flushed by the app, or triggered by input
- Mouse position tracked via interrupt + user input task

---

## 🔊 Audio Subsystem

### Deodhai Audio Server
```c
AudioTrack* audio_load(const char* path);
void audio_play(AudioTrack*);
void audio_stop(AudioTrack*);
```

- Audio server runs in userspace and interfaces via ring buffer or streaming interface
- Uses PCI device IDs to enumerate sound cards (Intel HDA)



---

### 🔊 Audio Services

#### `play_sound(void* buffer, int size)`
- Plays a block of PCM audio from memory.
- This is a system call that streams raw audio bytes to the sound hardware.
- Typically used with 16-bit signed PCM data (e.g., from MP3 decode).
- **Used in**: AudioPlayer app

---

### 📁 File I/O Utilities (Used by AudioPlayer)

#### `open(const char* path, int mode)`
- Opens a file and returns a file descriptor.
- Mode `0` is typically read-only.

#### `read(int fd, void* buf, int len)`
- Reads `len` bytes from the file into `buf`.

#### `close(int fd)`
- Closes the file associated with the descriptor.

---

### 🧠 Memory Management

#### `malloc(size_t size)` / `free(void* ptr)`
- Standard memory allocation functions.
- Used to allocate decode buffers or dynamic state (e.g., `mp3_decoder_t`)

---

### 🎧 Audio Decoder Integration (Userland)

While not a kernel service, the `minimp3` decoder is a userland library linked with the app. The audio output from this decoder is streamed into `play_sound()`.
---

## 🌐 Networking (NetManager)

### Overview
- Device: Intel E1000 (PCI-mapped)
- Stack: Custom NIC layer, raw packet parsing
- Planned: full TCP/IP with ARP, DHCP, socket-style API

---

## 🧪 Example: File-Reading App

```c
#include <xeclib.h>

int main() {
    int fd = file_open("/sys/readme.txt");
    char buf[128];
    file_read(fd, buf, 127);
    buf[127] = '\0';
    printf("Contents: %s\n", buf);
    file_close(fd);
    return 0;
}
```

---

## 📤 System Calls Summary

| Syscall         | Purpose                   |
|----------------|---------------------------|
| `file_open`     | Open a file               |
| `file_read`     | Read bytes from file      |
| `file_write`    | Write bytes to file       |
| `malloc` / `free` | Memory allocation      |
| `send_message`  | IPC to another process    |
| `create_window` | Create GUI window         |

---

## 📌 Future Kernel Services

- Threading API
- Shared memory regions
- Device I/O ports + MMIO abstraction
- Sandboxed syscall sets per process
- Real-time scheduler profiles

---

This file supports AI agents or human developers building deeper system-layer tools and apps on XenevaOS.
