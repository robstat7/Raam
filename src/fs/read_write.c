#include <raam/fs.h>
#include <raam/nvme.h>
#include <raam/printk.h>
#include <lib/string.h>

int sys_write(const int inode_nr, char *buf, int count)
{
	char *nvme_buffer = nvme_read(starting_sector, 8);	/* max 8 blocks only */

	/* block bitmap read */
	/* using uint16_t to support 16 blocks' tracking at the moment. */
	uint16_t *block_bitmap = (uint16_t *) (nvme_buffer + (BLOCK_BITMAP_BLOCK_NR * BLOCK_SIZE));

	uint16_t block_bitmap_val = *block_bitmap;

	printk("sys_write: block bitmap val read = {p}  ", (void *) block_bitmap_val);


	/* block bitmap write */

	struct superblock_struct *super = (struct superblock_struct *) nvme_buffer;

	block_bitmap_val =  block_bitmap_val | pow(2, super->next_free_block);

	*block_bitmap = block_bitmap_val;

	printk("sys_write: block bitmap val write = {p}  ", (void *) block_bitmap_val);

	super->next_free_block += 1; /* increment next free block */

	/* inode table write */
	struct inode_struct *inode = (struct inode_struct *) (nvme_buffer + (INODE_TABLE_BLOCK_NR * BLOCK_SIZE));	

	inode[inode_nr].size = count;
	inode[inode_nr].data_block_num = super->next_free_block - 1;

	printk("@sys_write: data_block_num = {d}  ", inode[inode_nr].data_block_num);

	char *data_block = NULL;

	/* data block write */
	if(inode[inode_nr].data_block_num <= 7) {	// block nr. 7 for 4th inode
		data_block = nvme_buffer + (inode[inode_nr].data_block_num * BLOCK_SIZE);
		strncpy(data_block, buf, count);
		nvme_write(starting_sector, 8);

	} else {
		nvme_write(starting_sector, 8);	/* write block bitmap and inode table */
		/* read from block nr. 8 (block for 5th inode data). read 8 blocks */
		nvme_buffer = nvme_read(starting_sector + 8, 8);
		data_block = nvme_buffer;
		strncpy(data_block, buf, count);
		nvme_write(starting_sector + 8, 8);
	}

}	

int sys_read(const int inode_nr, char *buf, int count)
{
	char *nvme_buffer = nvme_read(starting_sector, 4);
	
	/* inode table read */
	struct inode_struct *inode = (struct inode_struct *) (nvme_buffer + (INODE_TABLE_BLOCK_NR * BLOCK_SIZE));	

	uint8_t data_block_num = inode[inode_nr].data_block_num;
	char *data_block = NULL;

	printk("@sys_read: data_block_num = {d}  ", data_block_num);

	/* data block read */
	if(data_block_num <= 7) {
		nvme_buffer = nvme_read(starting_sector, 8);
		data_block = nvme_buffer + (data_block_num * BLOCK_SIZE);
	} else if (data_block_num > 7) {
		/* read from block nr. 8 (block for 5th inode data). read 8 blocks */
		nvme_buffer = nvme_read(starting_sector + 8, 8);
		data_block = nvme_buffer;
	}

	strncpy(buf, data_block, count);
}



// calculate base^exponent
int pow(int base, int exponent)
{
	int result = 1;

	for(int i = 0; i < exponent; i++) {
		result *= base;
	}		

	return result;
}

