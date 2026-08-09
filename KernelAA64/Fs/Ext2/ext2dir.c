#include <Fs/vfs.h>
#include <Fs/vdisk.h>
#include <Fs/Ext2/ext2file.h>
#include <Fs/Ext2/ext2.h>
#include <string.h>
#include <Mm/pmmngr.h>
#include <aucon.h>
#include <_null.h>


#define EXT2_DIR_REC_LEN(name_len) (((8 + (name_len)) + 3) & ~3)


uint32_t Ext2AllocInode(Ext2Fs* fs) {
    if (!fs || !fs->superblock) return 0;

    if (fs->superblock->free_inodes_count == 0) {
        AuTextOut("[Ext2]: no free inodes available on volume.\r\n");
        return 0;
    }

    uint32_t block_size = fs->block_size;
	uint32_t sector_per_block = block_size / 512;
	uint32_t group_count = fs->block_group_count;

    uint8_t* bitmap_buffer = (uint8_t*)P2V((uint64_t)AuPmmngrAlloc());
    if (!bitmap_buffer) return 0;

    uint32_t allocated_inode_id = 0;
	uint32_t target_group = 0;

    for (uint32_t g = 0; g < group_count; g++) {
        if (fs->block_desc->free_inodes_count == 0) continue;

        uint32_t bitmap_block = fs->block_desc->inode_bitmap;
        uint32_t bitmap_lba = (uint64_t)bitmap_block * sector_per_block;

        memset(bitmap_buffer, 0, block_size);
        AuVDiskRead((AuVDisk*)fs->vdisk, bitmap_lba, sector_per_block, (uint64_t*)bitmap_buffer);

        uint32_t bytes_to_scan = fs->inodes_per_group / 8;
		if (bytes_to_scan > block_size) bytes_to_scan = block_size;

        for (uint32_t byte_index = 0; byte_index < bytes_to_scan; byte_index++) {
            if (bitmap_buffer[byte_index] == 0xFF) continue;
            for (uint8_t bit_index = 0; bit_index < 8; bit_index++) {
                if (!(bitmap_buffer[byte_index] & (1 << bit_index))) {
                    uint32_t relative_inode = (byte_index * 8) + bit_index + 1;
					allocated_inode_id = (g * fs->inodes_per_group) + relative_inode;

					bitmap_buffer[byte_index] |= (1 << bit_index);

					AuVDiskWrite((AuVDisk*)fs->vdisk, bitmap_lba, sector_per_block, (uint64_t*)bitmap_buffer);
					target_group = g;
					goto success;
                }
            }
        }
    }
success:
    AuPmmngrFree((void*)V2P((uint64_t)bitmap_buffer));

	if (allocated_inode_id == 0) return 0;

	// Update metadata counters
	fs->superblock->free_inodes_count--;
	fs->block_desc[target_group].free_inodes_count--;

	Ext2FlushSuperblock(fs);
	Ext2FlushBgdt(fs);

	return allocated_inode_id;
};

int Ext2AddDirEntry(Ext2Fs* fs, Ext2Inode* parent_inode, uint32_t parent_inode_num, uint32_t child_inode_num, const char* child_name, uint8_t file_type) {
	if (!fs || !parent_inode || !child_name) return -1;

	uint32_t block_size = fs->block_size;
	uint32_t sector_per_block = block_size / 512;
	uint32_t name_len = strlen(child_name);
	uint16_t required_rec_len = EXT2_DIR_REC_LEN(name_len);

	uint8_t* block_buf = (uint8_t*)P2V((uint64_t)AuPmmngrAlloc());
	if (!block_buf) return -1;

	for (uint32_t i = 0; i < 12; i++) {
		uint32_t physical_block = parent_inode->block[i];

		if (physical_block == 0) {
			physical_block = Ext2AllocBlock(fs);
			if (physical_block == 0) {
				AuPmmngrFree((void*)V2P((uint64_t)block_buf));
				return -1;
			}

			parent_inode->block[i] = physical_block;
			parent_inode->size += block_size;
			parent_inode->blocks = (parent_inode->size + 511) / 512;
			Ext2InodeWrite(fs, parent_inode_num, parent_inode);

			memset(block_buf, 0, block_size);
			Ext2Dir* new_entry = (Ext2Dir*)block_buf;
			new_entry->inode = child_inode_num;
			new_entry->rec_len = block_size;
			new_entry->name_len = name_len;
			new_entry->file_type = file_type;
			memcpy(new_entry->name, (void*)child_name, name_len);

			AuVDiskWrite((AuVDisk*)fs->vdisk, (uint64_t)physical_block * sector_per_block, sector_per_block, (uint64_t*)block_buf);
			AuPmmngrFree((void*)V2P((uint64_t)block_buf));
			return 0;
		}

		uint64_t target_lba = (uint64_t)physical_block * sector_per_block;
		memset(block_buf, 0, block_size);
		AuVDiskRead((AuVDisk*)fs->vdisk, target_lba, sector_per_block, (uint64_t*)block_buf);

		uint32_t current_pos = 0;
		while (current_pos < block_size) {
			Ext2Dir* entry = (Ext2Dir*)(block_buf + current_pos);

			if (entry->rec_len == 0) break;

			uint16_t actual_rec_len = EXT2_DIR_REC_LEN(entry->name_len);

			if (entry->rec_len - actual_rec_len >= required_rec_len) {
				uint16_t old_rec_len = entry->rec_len;

				entry->rec_len = actual_rec_len;

				Ext2Dir* new_entry = (Ext2Dir*)(block_buf + current_pos + actual_rec_len);
				new_entry->inode = child_inode_num;
				new_entry->rec_len = old_rec_len - actual_rec_len;
				new_entry->name_len = name_len;
				new_entry->file_type = file_type;
				memcpy(new_entry->name, (void*)child_name, name_len);

				AuVDiskWrite((AuVDisk*)fs->vdisk, target_lba, sector_per_block, (uint64_t*)block_buf);
				AuPmmngrFree((void*)V2P((uint64_t)block_buf));
				return 0;
			}

			current_pos += entry->rec_len;
		}
	}

	AuPmmngrFree((void*)V2P((uint64_t)block_buf));
	return -1;
};

int Ext2Mkdir(AuVFSNode* parent_node, const char* name, uint16_t permissions) {
	if (!parent_node || !name) return -1;

	Ext2Fs* fs = (Ext2Fs*)parent_node->device;
	Ext2Inode* parent_inode = (Ext2Inode*)parent_node->private_data;

	if (!fs || !parent_inode) return -1;

	if (!(parent_node->flags & FS_FLAG_DIRECTORY)) {
		AuTextOut("[Ext2 Mkdir]: Parent node is not a directory.\r\n");
		return -1;
	}

	if (Ext2FindEntry(fs, parent_inode, name) != 0) {
		AuTextOut("[Ext2 Mkdir]: Directory entry already exists.\r\n");
		return -1;
	}

	uint32_t new_inode_num = Ext2AllocInode(fs);
	if (new_inode_num == 0) return -1;

	uint32_t new_block_num = Ext2AllocBlock(fs);
	if (new_block_num == 0) return -1;

	uint32_t block_size = fs->block_size;
	uint32_t sector_per_block = block_size / 512;

	uint8_t* dir_block = (uint8_t*)P2V((uint64_t)AuPmmngrAlloc());
	if (!dir_block) return -1;

	memset(dir_block, 0, block_size);

	Ext2Dir* dot = (Ext2Dir*)dir_block;
	dot->inode = new_inode_num;
	dot->rec_len = 12;
	dot->name_len = 1;
	dot->file_type = 2;
	dot->name[0] = '.';

	Ext2Dir* dotdot = (Ext2Dir*)(dir_block + 12);
	dotdot->inode = parent_node->first_block;
	dotdot->rec_len = block_size - 12;
	dotdot->name_len = 2;
	dotdot->file_type = 2;
	dotdot->name[0] = '.';
	dotdot->name[1] = '.';

	AuVDiskWrite((AuVDisk*)fs->vdisk, (uint64_t)new_block_num * sector_per_block, sector_per_block, (uint64_t*)dir_block);
	AuPmmngrFree((void*)V2P((uint64_t)dir_block));

	Ext2Inode new_inode;
	memset(&new_inode, 0, sizeof(Ext2Inode));

	new_inode.mode = EXT2_S_IFDIR | (permissions & 0x0FFF);
	new_inode.size = block_size;
	new_inode.links_count = 2;
	new_inode.blocks = block_size / 512;
	new_inode.block[0] = new_block_num;

	if (Ext2InodeWrite(fs, new_inode_num, &new_inode) != 0) return -1;

	if (Ext2AddDirEntry(fs, parent_inode, parent_node->first_block, new_inode_num, name, 2) != 0) return -1;

	parent_inode->links_count++;
	Ext2InodeWrite(fs, parent_node->first_block, parent_inode);

	uint32_t group = (new_inode_num - 1) / fs->inodes_per_group;
	fs->block_desc[group].used_dirs_count++;
	Ext2FlushBgdt(fs);

	return 0;
}

AuVFSNode* Ext2CreateDir(AuVFSNode* parent, char* name) {
	if (!parent || !name) return NULL;

	if (Ext2Mkdir(parent, name, 0755) != 0) {
		AuTextOut("[Ext2]: Failed to create directory on disk.\r\n");
		return NULL;
	}

	return Ext2Open(parent, name);
}