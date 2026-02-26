# Virtual File System Layer in XenevaOS
The Virtual File System (VFS) acts as an abstraction layer between applications and the underlying file system implementations for various applications like storage, device management in memory, etc. It provides a consistent, interface for file and directory operations such as open, read, write,close,etc. The VFS layer allows to mount multiple file system to the systems for example, block-based disk file systemms, memory-backed filesystems, and device or pseudo file systems. 

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
| ``fileCopyCount`` | Used in files,describes number of total copies of this file, this file is not closed until it reaches to zero |
| ``open`` | Used in file systems, used for open callback |
| ``opendir`` | Used in file systems, used for open a directory callback |
| ``read`` | Used in file systems, used for read callback |
| ``write`` | Used in file systems, used for write callback |
| ``create_dir`` | Used in file systems, used for creating a directory |
| ``remove_dir`` | Used in file systems, used for removing a directory |
| ``remove_file`` | Used in file systems, used for removing file |
| ``close`` | Used both in file systems and file, closing a specific file involves calling cleaner of that file, in case of file system, it unmounts the file system after cleaning every data structures of the file systems |
| ``read_block`` | Used in filesystems, used for reading a single block, depends mostly in the file system configuration, whether it uses 4KiB block size, or other else |
| ``read_dir`` | Used in filesystems, usef for reading entries of a directory |
| ``get_blockfor`` | Used in filesystems, used for getting the desired block address, of given file position in bytes |
| ``iocontrol`` | Used both in filesystems and files for controlling the filesystem or file |

## Implementing File System Driver for XenevaOS

In XenevaOS, File System driver can be implemented in two ways, either by implementing it as an external kernel module or by implementing it with the kernel code. But current version of kernel, lacks the process of installing file system driver as external kernel module. Current version of XenevaOS, only uses FAT32 as root file system implemented within the kernel code. Capability of the kernel to install file system from external kernel module is work in progress. <br>

### Mounting the file system during initialization
Mounting a new file system involves extracting all necessary information from its disk's root sector and creating necessary data structure and a new ``AuVFSNode`` structure with all file system information. File System driver can use ``AuVDisk`` service to read the disk sector. XenevaOS, firstly detects the storage medium connected to it using appropriate driver, then extract necessary information, and call AuVDisk to create an appropriate vdisk for it and mount the disk to device filesystem, and call appropriate file system mount it on that disk. 
```
Storage Disk --> Approriate Driver --> VDisk registration --> 
FileSystemInitialization(AuVDisk* vdisk, char* mountpoint);
//@param vdisk -- passed by storage driver,@param mountpoint -- passed by driver
```

| Function name | Description |
|---------------|-------------|
| ``AuAddFileSystem(AuVFSNode* node)`` | Used to add the file system to VFS layer |
| ``AuVFSRegisterRoot(AuVFSNode* node)`` | Used to register a file system as a root file system |
| ``AuDevFSAddFile(AuVFSNode* fs, char* path, AuVFSNode* file)`` | Used to add a device file to device file system, where fs points to device file system struct, path is the path to device file for exmple -- ``dev/nvme00``, file points to the device file |
| ``AuDevFSOpen(AuVFSNode* fs, char* path)`` | Used to open a device file respected to the path, for example -- ``/dev/ttym0`` |
| ``AuVFSFind(char* fsname)`` | Used to find a file system, from file system mounted list, for example - ``AuVFSFind("/dev")`` will return the device file system structure |
| ``AuCreateVDisk`` | Creates a virtual disk service |
| ``AuVDiskRegister`` | Register a new virtual disk to vdisk layer |
| ``AuVDiskGetIndex`` | Returns a index for new virtual disk |


## Virtual Disk Service in XenevaOS
Virtual Disk Service acts as single interface for multiple storage medium, providing robust and similar function calls for all type of storage medium unlike of its block size. Whenever new disk is detected, it immediately calls the vdisk service to register the new disk along with mounting the disk to device file system. It uses vdisk index number to number the disk within the same controller, for example an AHCI controller may have multiple SATA disk attached to it, which can be numbered as ``/dev/ahci/sata00``, ``/dev/ahci/sata01``, etc. 

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
|``diskname[40]``| name of the disk limited upto 40 character |
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