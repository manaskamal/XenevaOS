#include <Fs/vfs.h>
#include <fs/vdisk.h>
#include <Mm/kmalloc.h>
#include <Mm/pmmngr.h>
#include <Drivers/uart.h>
#include <string.h>
#include <aucon.h>
#include <Fs/Ext2/Ext2.h>
#include <_null.h>


AuVFSNode* Ext2Initialise(AuVDisk* vdisk, char* mountname) {
	uint64_t* buffer = (uint64_t*)AuPmmngrAlloc();
	memset(buffer, 0, 4096);

	AuVDiskRead(vdisk, 2, 2, buffer);

	Ext2Fs* fs = (Ext2Fs*)kmalloc(sizeof(Ext2Fs));
	memset(fs, 0, sizeof(Ext2Fs));
	fs->vdisk = (AuVFSNode*)vdisk;

	fs->superblock = (Ext2Superblock*)kmalloc(sizeof(Ext2Superblock));
	memcpy(fs->superblock, buffer, sizeof(Ext2Superblock));

	if (fs->superblock->magic != EXT2_SUPER_BLOCK_MAGIC) {
		AuPmmngrFree(buffer);
		kfree(fs->superblock);
		kfree(fs);
		return NULL;
	}

	fs->block_size = 1024 << fs->superblock->log_block_size;
	fs->pointers_per_block = fs->block_size / 4;
	fs->inode_size = (fs->superblock->inode_size == 0) ? 128 : fs->superblock->inode_size;

	fs->block_group_count = fs->superblock->blocks_count / fs->superblock->blocks_per_group;
	if ((fs->superblock->blocks_per_group * fs->block_group_count) < fs->superblock->blocks_count) {
		fs->block_group_count += 1;
	}
	fs->inodes_per_group = fs->superblock->inodes_count / fs->block_group_count;

	AuTextOut("[aurora]: EXT2 Volume Label: %s \r\n", fs->superblock->volume_name);
	AuTextOut("[aurora]: ext2 block size: %d bytes \r\n", fs->block_size);
	AuTextOut("[aurora]: ext2 inodes per group: %d \r\n", fs->inodes_per_group);
	AuTextOut("[aurora]: ext2 mounted at %s \r\n", mountname);

	fs->bgd_offset = (fs->block_size > 1024) ? 1 : 2;

	uint32_t bgdt_bytes = fs->block_group_count * sizeof(Ext2BlockDescriptor);
	fs->bgd_block_span = bgdt_bytes / fs->block_size;
	if ((fs->bgd_block_span * fs->block_size) < bgdt_bytes) {
		fs->bgd_block_span += 1;
	}

	fs->block_desc = (Ext2BlockDescriptor*)kmalloc(fs->block_size * fs->bgd_block_span);
	memset(fs->block_desc, 0, fs->block_size * fs->bgd_block_span);

	uint32_t sectors_per_block = fs->block_size / 512;
	for (uint32_t i = 0; i < fs->bgd_block_span; ++i) {
		memset(buffer, 0, 4096);
		uint64_t target_lba = (fs->bgd_offset + i) * sectors_per_block;

		AuVDiskRead(vdisk, target_lba, sectors_per_block, buffer);

		memcpy((uint8_t*)fs->block_desc + (i * fs->block_size), buffer, fs->block_size);
	}

	AuVFSNode* fsys = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(fsys, 0, sizeof(AuVFSNode));

	strcpy(fsys->filename, mountname);
	fsys->flags |= FS_FLAG_FILE_SYSTEM | FS_FLAG_DIRECTORY;
	fsys->first_block = 2;
	fsys->device = fs;

	fsys->open = NULL;
	fsys->read = NULL;
	fsys->read_dir = NULL;

	fsys->write = NULL;
	fsys->create_dir = NULL;
	fsys->create_file = NULL;

	vdisk->fsys = fsys;
	fs->root_node = fsys;

	AuVFSAddFileSystem(fsys);
	AuVFSRegisterRoot(fsys);

	AuPmmngrFree(buffer);

	return fsys;
}