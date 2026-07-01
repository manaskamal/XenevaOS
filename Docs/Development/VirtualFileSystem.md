# XenevaOS Virtual File System (VFS)

The Virtual File System (VFS) is a critical abstraction layer in the Aurora kernel of XenevaOS. It allows the kernel to treat different types of file storage—whether they are physical disk partitions, hardware devices, or memory-resident filesystems—under a unified interface of generic file operations.

---

## 1. VFS Mount Point Rules

XenevaOS differentiates between **General (Block-based) Filesystems** (such as FAT32, Ext2) and **Virtual/Memory-based Filesystems** (such as `/dev`, `/proc`, `/tty`).

### Mount Point Letter Reservation
For general, block-backed filesystems that require mounting to a specific drive/volume hierarchy:
* **Function:** Always call `AuVFSReserveMountPointLetter()` during mounting to reserve a mount letter.
* **Limitation:** This function is **only** applicable to general filesystems.
* **Prohibition:** Do **NOT** use `AuVFSReserveMountPointLetter()` for virtual or memory-based filesystems like `/proc`, `/tty`, or `/dev`. These are mounted directly to fixed virtual paths within the VFS structure during early kernel boot and do not require volume letter mapping.

---

## 2. Writing Your Own File System Module Inside the Kernel

To implement a filesystem driver inside the Aurora kernel, you must define its operations, map them to VFS node callbacks, and register it.

### Step 1: Define File Operation Callbacks
You must implement a set of filesystem callback functions that conform to the VFS callback signatures defined in `Fs/vfs.h`:

```c
#include <Fs/vfs.h>

// Example: Callback to read from your filesystem
size_t MyFSRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length) {
    // 1. Calculate offset and disk sector/block
    // 2. Fetch data from storage media
    // 3. Write data to the destination 'buffer'
    // 4. Return bytes read
    return length;
}

// Example: Callback to write to your filesystem
size_t MyFSWrite(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length) {
    // 1. Locate blocks on device
    // 2. Write data from buffer to media
    // 3. Update file size and position
    return length;
}
```

### Step 2: Initialize the VFS Node (`AuVFSNode`)
When your filesystem is initialized or a file is opened, you must construct an `AuVFSNode` structure and populate its properties and function pointers:

```c
AuVFSNode* my_fs_node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
memset(my_fs_node, 0, sizeof(AuVFSNode));

// Set basic metadata
strcpy(my_fs_node->filename, "myfs");
my_fs_node->size = 0;
my_fs_node->flags = FS_FLAG_FILE_SYSTEM | FS_FLAG_FILE_SYSTEM_GENERAL;

// Bind callbacks
my_fs_node->read = MyFSRead;
my_fs_node->write = MyFSWrite;
my_fs_node->open = MyFSOpen;
my_fs_node->close = MyFSClose;
my_fs_node->read_dir = MyFSReadDir;
my_fs_node->create_file = MyFSCreateFile;
my_fs_node->create_dir = MyFSCreateDir;
```

#### Node Flags (`flags`)
Ensure you set the appropriate flags so the VFS handles the node correctly:
* `FS_FLAG_DIRECTORY`: Node is a directory.
* `FS_FLAG_FILE_SYSTEM_GENERAL`: Standard general-purpose block filesystem.
* `FS_FLAG_DEVICE`: Virtual device node (like nodes inside `/dev`).
* `FS_FLAG_PIPE`: Inter-process communication pipe.

### Step 3: Register the Filesystem
To make your filesystem visible to the kernel and userspace applications, register it with the VFS:

* **For Root Filesystems:**
  Use `AuVFSRegisterRoot(my_fs_node)` to mount the filesystem as the root partition (`/`).
* **For Sub-Filesystems / General Mounts:**
  Use `AuVFSAddFileSystem(my_fs_node)` to register the filesystem node under the standard VFS list.
