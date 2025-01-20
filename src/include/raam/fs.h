#ifndef FS_H
#define FS_H

#include <stdint.h>

#define BLOCK_SIZE		512	/* 512 bytes */

#define MAGIC			0x6d616172	/* raam */

#define INODE_BITMAP_BLOCK_NR           2                                       
#define BLOCK_BITMAP_BLOCK_NR           1                                       
#define INODE_TABLE_BLOCK_NR            3

#define FILE_MAX_SIZE			512	/* in bytes */

struct superblock_struct {
	uint32_t magic;
	uint16_t block_size;
	uint8_t total_blocks;
	uint8_t total_data_blocks;
	uint16_t total_inodes;
	uint8_t first_data_block;
	uint8_t next_free_block;	/* next free data block */
	uint16_t file_max_size;		/* in bytes */
}__attribute__((packed));

struct inode_struct {
	char name[11];	/* file name len = 10 */
	uint16_t size;	/* in bytes */
	uint8_t data_block_num;	/* only single data block */
}__attribute__((packed));

extern uint32_t starting_sector;

void sys_creat(const char *filename);
int sys_open(const char *filename);
int sys_write(const int inode_nr, char *buf, int count);
int sys_read(const int inode_nr, char *buf, int count);

int pow(int base, int exponent);
int count_on_bits(unsigned char byte);

#endif	/* FS_H */
