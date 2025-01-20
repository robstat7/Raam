#include <stdint.h>
#include <raam/nvme.h>
#include <raam/fs.h>
#include <raam/printk.h>
#include <lib/string.h>

#define INODE_BITMAP_BLOCK_NR		2
#define INODE_TABLE_BLOCK_NR		3

/*
 * starting sector of the Raam filesystem formatted SSD partition.
 * Note: I'm hardcoding it now for simplicity. Be cautious with this
 * number as writing to the WRONG LOCATION CAN CORRUPT the data on disk.
 * TODO: Later, I will implement the logic in bootloader to find this
 * starting sector by checking the magic of the filesystem layout in
 * the superblock.
 */
const uint32_t starting_sector = 997109760;	/* CHECK IT CAREFULLY!!! */

void sys_creat(const char *pathname)
{
	/* read superblock block #0 (512 bytes) */
	char *nvme_buffer = nvme_read(starting_sector, 1);

	struct superblock_struct *super = (struct superblock_struct *) nvme_buffer;

	uint8_t total_blocks = super->total_blocks;

	printk("sys_creat: total blocks = {d}  ", total_blocks);

	nvme_buffer = nvme_read(starting_sector, 4);	/* read 4 blocks into the nvme buffer for metadata */

	/* find the first free inode */	

	/* inode bitmap read */
	uint8_t *inode_bitmap = (uint8_t *) (nvme_buffer + (INODE_BITMAP_BLOCK_NR * BLOCK_SIZE));

	uint8_t inode_bitmap_byte_0_val = *inode_bitmap;

	printk("sys_creat: inode bitmap byte 0 val read = {p}  ", (void *) inode_bitmap_byte_0_val);

	/* write inode bitmap */

	inode_bitmap_byte_0_val |= 0x1;
	
	printk("sys_creat: inode bitmap byte 0 val to write = {p}  ", (void *) inode_bitmap_byte_0_val);

	*inode_bitmap = inode_bitmap_byte_0_val;

	/* write the inode table */
	struct inode_struct *inode = (struct inode_struct *) (nvme_buffer + (INODE_TABLE_BLOCK_NR * BLOCK_SIZE));

	strncpy(inode->name, pathname, strlen(pathname));

	inode->size = 0;	/* we are only creating a file with no contents */

	inode->data_block_num = 0;	/* storing invalid value */

	/* write nvme buffer to the partition */
	nvme_write(starting_sector, 4);
}
