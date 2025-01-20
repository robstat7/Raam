#ifndef FS_H
#define FS_H

#include <stdint.h>

#define BLOCK_SIZE		512	/* 512 bytes */

#define MAGIC			0x6d616172	/* raam */

struct superblock_struct {
	uint32_t magic;
	uint16_t block_size;
	uint8_t total_blocks;
	uint8_t total_data_blocks;
	uint16_t total_inodes;
	uint8_t first_data_block;
}__attribute__((packed));

struct inode_struct {
	char name[11];	/* file name len = 10 */
	uint16_t size;	/* in bytes */
	uint8_t data_block_num;
}__attribute__((packed));

void sys_creat(const char *filename);
void sys_open(const char *filename);

#endif	/* FS_H */
