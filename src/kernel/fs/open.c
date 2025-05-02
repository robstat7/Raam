#include <lib/string.h>
#include <raam/printk.h>
#include <raam/fs/open.h>
#include <raam/nvme.h>

const uint32_t raam_root_starting_sector = 997109760;

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
