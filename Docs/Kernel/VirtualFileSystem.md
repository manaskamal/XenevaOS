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



