#ifndef OPEN_H
#define OPEN_H

#define EXT2_BLOCK_SIZE		4096	/* in bytes */

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
	uint32_t	bg_block_bitmap;		/* Blocks bitmap block */
	uint32_t	bg_inode_bitmap;		/* Inodes bitmap block */
	uint32_t	bg_inode_table;		/* Inodes table block */
	uint16_t	bg_free_blocks_count;	/* Free blocks count */
	uint16_t	bg_free_inodes_count;	/* Free inodes count */
	uint16_t	bg_used_dirs_count;	/* Directories count */
	uint16_t	bg_pad;
	uint32_t	bg_reserved[3];
};

int sys_open(const char *filename, int flag, int mode);
static void get_only_file_name(const char *filename, const char *only_file_name);
static uint32_t get_inode_table_start_block(void);

#endif	/* OPEN_H */
