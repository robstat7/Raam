#include <raam/fs.h>
#include <raam/nvme.h>
#include <raam/printk.h>
#include <lib/string.h>

int sys_write(const int inode_nr, char *buf, int count)
{
	char *nvme_buffer = nvme_read(starting_sector, 6);	

	/* block bitmap read */
	uint8_t *block_bitmap = (uint8_t *) (nvme_buffer + (BLOCK_BITMAP_BLOCK_NR * BLOCK_SIZE));

	uint8_t block_bitmap_byte_0_val = *block_bitmap;

	printk("sys_write: block bitmap byte 0 val read = {p}  ", (void *) block_bitmap_byte_0_val);

	struct superblock_struct *super = (struct superblock_struct *) nvme_buffer;

	printk("sys_write: first data block nr = {d}  ", super->first_data_block);

	block_bitmap_byte_0_val =  block_bitmap_byte_0_val | ((uint8_t) pow(2, super->next_free_block));

	*block_bitmap = block_bitmap_byte_0_val;

	printk("sys_write: block bitmap byte 0 val write = {p}  ", (void *) block_bitmap_byte_0_val);

	super->next_free_block += 1; /* increment next free block */

	/* inode table write */
	struct inode_struct *inode = (struct inode_struct *) (nvme_buffer + (INODE_TABLE_BLOCK_NR * BLOCK_SIZE));	

	inode[inode_nr].size = count;
	inode[inode_nr].data_block_num = super->next_free_block - 1;

	/* data block write */
	char * data_block = nvme_buffer + (inode[inode_nr].data_block_num * BLOCK_SIZE);

	strncpy(data_block, buf, count);
	
	nvme_write(starting_sector, 6);

}	

int sys_read(const int inode_nr, char *buf, int count)
{
	char *nvme_buffer = nvme_read(starting_sector, 6);	
	
	/* inode table read */
	struct inode_struct *inode = (struct inode_struct *) (nvme_buffer + (INODE_TABLE_BLOCK_NR * BLOCK_SIZE));	

	uint8_t data_block_num = inode[inode_nr].data_block_num;

	/* data block read */
	char *data_block = nvme_buffer + (data_block_num * BLOCK_SIZE);

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

