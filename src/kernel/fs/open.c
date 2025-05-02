#include <lib/string.h>
#include <raam/printk.h>
#include <raam/fs/open.h>
#include <raam/nvme.h>

const uint32_t raam_root_starting_sector = 997109760;

#define INODE_SIZE	256	/* in bytes */

uint32_t find_file(const uint8_t *buf, const char *file_name);

int sys_open(const char *filename, int flag, int mode)
{
	char only_file_name[100];

	get_only_file_name(filename, only_file_name);

	printk("only_file_name: ");
	printk(only_file_name);
	printk("  ");

	/* get inode table start block address for block group 0 */

	uint32_t bg_inode_table = get_inode_table_start_block();

	printk("inode_table_start_block = {d}  ", bg_inode_table);


	/* read inode no.2 i.e. the root inode to get root data */

	uint32_t inode_table_sector = raam_root_starting_sector + (bg_inode_table * 8);

	char *buf = nvme_read(inode_table_sector, 1);

	struct ext2_inode *root_inode = (struct ext2_inode *) (buf + INODE_SIZE);

	uint32_t direct_block_pointer_0 = root_inode->i_block[0];

	printk("direct_block_pointer_0: {d}  ", direct_block_pointer_0); 

	
	/* read root dir data */
		
	uint32_t root_dir_data_sector = raam_root_starting_sector + (direct_block_pointer_0 * 8);
	buf = nvme_read(root_dir_data_sector, 8);

	uint32_t file_inode = find_file((uint8_t *) buf, only_file_name);	

	if(file_inode != 0) {
		printk("found the file. Inode = {d}  ", file_inode);
	}

	return 1;
}

static void get_only_file_name(const char *filename, const char *only_file_name)
{
	strncpy(only_file_name, filename + 1, strlen(filename));
}

static uint32_t get_inode_table_start_block(void)
{
	/* block group desc table starts at block 1 */
	uint32_t group_desc_sector = raam_root_starting_sector + 8;
	/* read block group desc table for block group 0 */
	char *buf = nvme_read(group_desc_sector, EXT2_BLOCK_SIZE / 512);

	struct ext2_group_desc *desc = (struct ext2_group_desc *) buf;	

	return desc->bg_inode_table;
}

/* returns file inode or 0 if file not found */
uint32_t find_file(const uint8_t *buf, const char *file_name)
{
	uint8_t *ptr = buf;
	uint8_t *end = buf + 4096;	/* 1 block of data */	
	
	while (ptr < end) {
		struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *) ptr;

		/* skip deleted/unused inodes */
		if(entry->inode == 0) {
			ptr += entry->rec_len;
			continue;
		}

		char name[256];

		strncpy(name, entry->name, entry->name_len);

		name[entry->name_len] = '\0';

		if (strncmp(name, file_name, entry->name_len) == 0) {
			return entry->inode;
		}

		ptr += entry->rec_len;

	}

	return 0;
}
