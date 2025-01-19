#ifndef FS_H
#define FS_H

#include <stdint.h>

#define BLOCKS_PER_GROUP		256	/* for simplicity */

#define EXT2_LOG_BLOCK_SIZE		0	/* 1024 = 1024 << 0 */

#define EXT2_REV_LEVEL			0	/* revision 0 */

#define EXT2_SUPER_MAGIC		0xef53

struct superblock_struct {
	uint32_t s_inodes_count;
	uint32_t s_blocks_count;
	uint32_t s_r_blocks_count;
	uint32_t s_free_blocks_count;
	uint32_t s_free_inodes_count;
	uint32_t s_first_data_block;
	uint32_t s_log_block_size;
	uint32_t s_blocks_per_group;
	uint32_t s_inodes_per_group;
	char padding1[12];	/* padding to get the magic signature */
				/* at the same offset as in the */
				/* revision 1. */
	uint16_t s_magic;
	char padding2[18];	/* padding to get the revision level */
				/* at the same offset as in the */
				/* revision 1. */
	uint32_t s_rev_level;
}__attribute__((packed));

#endif	/* FS_H */
