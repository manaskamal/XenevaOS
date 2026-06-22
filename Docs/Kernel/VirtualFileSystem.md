# Virtual File System Layer in XenevaOS
The Virtual File System (VFS) acts as an abstraction layer between applications and the underlying filesystem implementations (such as storage, in-memory device management, etc.). It provides a consistent interface for file and directory operations like open, read, write, close, etc. The VFS layer allows mounting multiple filesystems to the system, such as block-based disk filesystems, memory-backed filesystems, and device or pseudo-filesystems. 

## Virtual File System Structure

This structure is used for describing both file and file systems.
```
typedef struct __VFS_NODE__ {
    char filename[32];
    uint32_t size;
    uint8_t eof;
    uint32_t pos;
    uint32_t parent_block;
    uint64_t first_block;
    uint64_t current;
    uint16_t flags;
    uint8_t status;
    void* device;
    uint16_t fileCopyCount;
    open_callback open;
    opendir_callback opendir;
    read_callback read;
    write_callback write;
    create_dir_callback create_dir;
    create_file_callback create_file;
    remove_dir_callback remove_dir;
    remove_file_callback remove_file
    close_callback close;
    read_block_callback read_block;
    readdir_callback read_dir;
    fs_getblockfor get_blockfor;
    iocontrol_callback iocontrol;
}
```
| Field name | Description |
|------------|-------------|
| ``filename[32]`` | Name of the file or filesystem, limited to 32 characters |
| ``size`` | The total size of the file or filesystem, mostly usable for file |
| ``eof`` | End-Of-File used in files, in order to describe if the file's read position marker has reached its end |
| ``pos`` | Current position in the file, used for reading and writing |
| ``parent_block`` | Used in files, describes the parent block address of the file, for example - directory cluster of the file in FAT32 fs |
| ``first_block`` | Used in files, describes the very first block address of the file, for example - first cluster of the file in FAT32 fs |
| ``current`` | Used in files, describes the current block address of the file, for example - current read cluster of the file in FAT32 fs |
| ``flags`` | Used both in files and file systems, describing the type of File or file system |
| ``status`` | Used in files, describing current condition of the file |
| ``device`` | Used both in files and filesystems, used for pointing file system's private data structure or file's private data structure |
| ``fileCopyCount`` | Used in files; describes the total copies of this file. The file is not closed until this reaches zero. |
| ``open`` | Used in file systems, used for open callback |
| ``opendir`` | Used in file systems, used for open a directory callback |
| ``read`` | Used in file systems, used for read callback |
| ``write`` | Used in file systems, used for write callback |
| ``create_dir`` | Used in file systems, used for creating a directory |
| ``remove_dir`` | Used in file systems, used for removing a directory |
| ``remove_file`` | Used in file systems, used for removing file |
| ``close`` | Used both in filesystems and files. Closing a specific file calls its cleanup handler. For a filesystem, it unmounts the filesystem after cleaning up its internal data structures. |
| ``read_block`` | Used in filesystems; reads a single block. Depends on the filesystem configuration (e.g., whether it uses 4 KiB block size or another size). |
| ``read_dir`` | Used in filesystems; used for reading the entries of a directory. |
| ``get_blockfor`` | Used in filesystems; retrieves the desired block address corresponding to a given file position in bytes. |
| ``iocontrol`` | Used both in filesystems and files for controlling the filesystem or file |

## Implementing File System Driver for XenevaOS

In XenevaOS, filesystem drivers can be implemented in two ways: either as external kernel modules or built directly into the kernel code. However, the current version of the kernel lacks the mechanism to load filesystem drivers as external kernel modules. The current version of XenevaOS only uses a built-in FAT32 driver for the root filesystem. Supporting filesystem drivers from external modules is work in progress. <br>

### Mounting the file system during initialization
Mounting a new filesystem involves extracting all necessary information from the disk's root sector, creating the necessary data structures, and allocating a new `AuVFSNode` structure. Filesystem drivers can use the `AuVDisk` service to read disk sectors. XenevaOS first detects the storage medium using the appropriate driver, extracts the partition details, calls `AuCreateVDisk` to register a virtual disk, mounts it to the device filesystem, and finally mounts the target filesystem on it. 
```
Storage Disk --> Approriate Driver --> VDisk registration --> 
FileSystemInitialization(AuVDisk* vdisk, char* mountpoint);
//@param vdisk -- passed by storage driver,@param mountpoint -- passed by driver
```

| Function name | Description |
|---------------|-------------|
| ``AuAddFileSystem(AuVFSNode* node)`` | Used to add the file system to VFS layer |
| ``AuVFSRegisterRoot(AuVFSNode* node)`` | Used to register a file system as a root file system |
| ``AuDevFSAddFile(AuVFSNode* fs, char* path, AuVFSNode* file)`` | Adds a device file to the device filesystem (where `fs` points to the device filesystem, `path` is the file path e.g., ``dev/nvme00``, and `file` points to the device file node). |
| ``AuDevFSOpen(AuVFSNode* fs, char* path)`` | Opens a device file corresponding to the path (e.g., ``/dev/ttym0``). |
| ``AuVFSFind(char* fsname)`` | Finds a filesystem from the mount list (e.g., ``AuVFSFind("/dev")`` returns the device filesystem node). |
| ``AuCreateVDisk`` | Creates a virtual disk instance. |
| ``AuVDiskRegister`` | Registers a new virtual disk to the vdisk layer. |
| ``AuVDiskGetIndex`` | Returns a unique index for a new virtual disk. |


## Virtual Disk Service in XenevaOS
The Virtual Disk Service acts as a unified interface for multiple storage media, providing robust, standardized function calls regardless of the block size of the underlying storage. Whenever a new disk is detected, the kernel registers it with the vdisk service and mounts it to the device filesystem. It uses a vdisk index to identify disks attached to the same controller (for example, multiple SATA disks on an AHCI controller may be numbered as `/dev/ahci/sata00`, `/dev/ahci/sata01`, etc.). 

### Virtual Disk structure
This describes the virtual disk.
```
typedef struct _VDISK_ {
    char diskname[40];
    char serialNumber[20];
    char diskPath[32];
    void* data;
    uint64_t max_blocks;
    uint64_t blockSize;
    uint8_t __VDiskID;

    // Partition specific information
    // AuPartition part[MAX_PARTITION_PER_DISK]
    // for now, only single partition is supported
    uint8_t num_partition;
    uint64_t startingLBA;
    uint64_t currentLBA;

    GUID part_guid;
    GUID part_unique_guid;
    // Pointer to mounted file system
    AuVFSNode* fsys;
    // disk specific
    vdisk_read Read;
    vdisk_write Write;

}AuVDisk;
```

| Field Name | Description |
|------------|-------------|
|``diskname[40]``| Name of the disk (limited to 40 characters). |
|``serialNumber[20]`` | serial number of the disk |
|``diskPath[32]`` | path of the disk in device file system |
|``data`` | Pointer to internal data structure of the disk |
|``max_blocks`` | Maximum number of blocks that this disk has | 
|``blockSize`` | Size of a block in this disk |
|``__VDiskID`` | Index of this vdisk, returned by ``AuVDiskGetIndex`` | 
|``num_partition`` | Number of partition this disk has |
|``startingLBA`` | Starting LBA block of this partition |
|``currentLBA`` | Don't know where to use this |
|``part_guid`` | GUID information of this partition |
|``part_unique_guid`` | Unique GUID information of this partition |
|``fsys`` | Pointer to mounted file system within this disk |
|``Read`` | Read callback for this virtual disk |
|``Write`` | Write callback for this virtual disk |

 Please refer to XenevaOS source code repository for more example for usage of all functions.