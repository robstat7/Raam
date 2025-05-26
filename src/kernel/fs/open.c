#include <lib/string.h>
#include <raam/printk.h>
#include <raam/fs/open.h>
#include <raam/nvme.h>
#include <stdint.h>
// #include <string.h>

const uint32_t raam_root_starting_sector = 997109760;

#define INODE_SIZE	256	/* in bytes */

int get_file_inode(const char *filename);
uint32_t get_inode_table_start_block_nr_for_group_0(void);
uint32_t find_file(const uint8_t *buf, const char *file_name);
int count_total_directories_in_path(const char *filename);
const char *find_next_slash(char *path);
void read_directories(const char *filename, const char *only_file_name, const int total_dirs, const uint32_t inode_table_start_block_nr);
void find_dir_name(const char *filename, const int dir_num, char *dir_name);
uint32_t get_directory_inode_nr(const char *dir_name, const int parent_dir_inode_nr, const uint32_t inode_table_start_block_nr);
uint32_t get_directory_inode_nr_from_parent_block(const uint32_t parent_dir_direct_block_pointer_0, const char *dir_name);
uint32_t get_parent_directory_data_block_nr(const int parent_dir_inode_nr, const uint32_t inode_table_start_block_nr);

int sys_open(const char *filename, int flag, int mode)
{
	get_file_inode(filename);	

	// /* read inode no. 2 i.e. the root inode to get root data */

	// uint32_t inode_table_sector = raam_root_starting_sector + (bg_inode_table * 8);

	// char *buf = nvme_read(inode_table_sector, 1);

	// struct ext2_inode *root_inode = (struct ext2_inode *) (buf + INODE_SIZE);

	// uint32_t direct_block_pointer_0 = root_inode->i_block[0];

	// printk("root direct_block_pointer_0: {d}  ", direct_block_pointer_0); 



	// /* read root dir data */

	// uint32_t root_dir_data_sector = raam_root_starting_sector + (direct_block_pointer_0 * 8);
	// buf = nvme_read(root_dir_data_sector, 8);

	// uint32_t file_inode = find_file((uint8_t *) buf, only_file_name);	

	// if(file_inode != 0) {
	// 	printk("found the file. Inode = {d}  ", file_inode);
	// }

	return 1;
}

static void get_only_file_name(const char *filename, const int total_dirs, const char *only_file_name)
{
	int i;
	
	char *path = filename;

	for (i = 0; i < total_dirs; i++) {
		if (i == total_dirs - 1) {
			strncpy(only_file_name, path + 1, strlen(path));
		}

		else {
			path = find_next_slash(path + 1);
		}
	}
}

const char *find_next_slash(char *path)
{
	int i;

	i = 0;

	while (path[i] != '\0') {
		if (path[i] == '/') {
			path = &path[i];
			break;
		}

		i++;
	}

	return path;
}

int count_total_directories_in_path(const char *filename)
{
	int i = 0;
	int total_dirs = 0;

	while (filename[i] != '\0') {
		if (filename[i++] == '/') {
			total_dirs++;
		}
	}

	return total_dirs;
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

int get_file_inode(const char *filename)
{
	int total_dirs;
	char only_file_name[100];
	uint32_t inode_table_start_block_nr;
	
	total_dirs = count_total_directories_in_path(filename);

	printk("total dirs in filename = {d}  ", total_dirs);
	
	get_only_file_name(filename, total_dirs, only_file_name);

	printk("only_file_name: ");
	printk(only_file_name);
	printk("  ");

	inode_table_start_block_nr = get_inode_table_start_block_nr_for_group_0();

	// printk("@@filename = ");
	// printk(filename);
	// printk("  ");

	read_directories(filename, only_file_name, total_dirs, inode_table_start_block_nr);

	return 0;
}

/* find directories and their inodes and read their data to find the file inode */
void read_directories(const char *filename, const char *only_file_name, const int total_dirs, const uint32_t inode_table_start_block_nr)
{
	char dir_name[100];
	uint32_t inode;
	int dir_num, parent_dir_inode_nr;
	
	dir_num = 2;
	parent_dir_inode_nr = 2;	/* root inode number */
	inode = parent_dir_inode_nr;
	while (dir_num <= total_dirs) {
		find_dir_name(filename, dir_num, dir_name);
		printk("dir_name {d} = ", dir_num);
		printk(dir_name);
		printk("  ");

		inode = get_directory_inode_nr(dir_name, parent_dir_inode_nr, inode_table_start_block_nr);
		printk("dir inode = {d}  ", inode);

		dir_num++;
	}

	get_file_inode_nr(only_file_name, inode, inode_table_start_block_nr);
}

/* get the inode number for the directory given by `dir_name` in its parent directory */
uint32_t get_directory_inode_nr(const char *dir_name, const int parent_dir_inode_nr, const uint32_t inode_table_start_block_nr)
{
	uint32_t parent_dir_direct_block_pointer_0, inode;

	parent_dir_direct_block_pointer_0 = get_parent_directory_data_block_nr(parent_dir_inode_nr, inode_table_start_block_nr);

	printk("parent_dir_direct_block_pointer_0: {d}  ", parent_dir_direct_block_pointer_0); 

	inode = get_directory_inode_nr_from_parent_block(parent_dir_direct_block_pointer_0, dir_name);

	return inode;
}

/* return non-zero inode number of the directory by reading the parent directory data block. */
/* returns zero if the directory is not found */
uint32_t get_directory_inode_nr_from_parent_block(const uint32_t parent_dir_direct_block_pointer_0, const char *dir_name)
{
	uint8_t *ptr;
	uint8_t *end;
	uint32_t parent_dir_data_sector;
	char *buf;
	struct ext2_dir_entry_2 *entry;
	char name[256];

	parent_dir_data_sector = raam_root_starting_sector + (parent_dir_direct_block_pointer_0 * 8);
	buf = nvme_read(parent_dir_data_sector, 8);
	
	ptr = buf;
	end = buf + 4096;	/* 1 block of data */
	while (ptr < end) {
		entry = (struct ext2_dir_entry_2 *) ptr;

		/* skip deleted/unused inodes */
		if(entry->inode == 0) {
			ptr += entry->rec_len;
			continue;
		}

		strncpy(name, entry->name, entry->name_len);

		name[entry->name_len] = '\0';

		printk(name);

		printk("  ");

		if (strncmp(name, dir_name, entry->name_len) == 0) {
			return entry->inode;
		}

		ptr += entry->rec_len;

	}

	return 0;
}

uint32_t get_parent_directory_data_block_nr(const int parent_dir_inode_nr, const uint32_t inode_table_start_block_nr)
{
	uint32_t inode_table_sector, direct_block_pointer_0;
	char *buf;
	struct ext2_inode *inode;
	
	inode_table_sector = raam_root_starting_sector + (inode_table_start_block_nr * 8);
	
	buf = nvme_read(inode_table_sector, 8);

	inode = (struct ext2_inode *) (buf + ((parent_dir_inode_nr - 1) * INODE_SIZE));

	direct_block_pointer_0 = inode->i_block[0];

	return direct_block_pointer_0;
}

/* find the directory name like '/', 'src', 'bin', etc */
void find_dir_name(const char *filename, const int dir_num, char *dir_name)
{
	int i, pos, len;

	/* directory is the root directory */
	if (dir_num == 1) {
		// strncpy(dir_name, "/\0", 2);
		dir_name[0] = '/';
		dir_name[1] = '\0';
		return;
	}

	i = 1;
	pos = 1;
	while (filename[i] != '\0') {
		if (filename[i] == '/') {
			len = i - pos;
			strncpy(dir_name, &filename[pos], len);
			dir_name[len] = '\0';
			return;
		}
		i++;
	}
}

/* get inode table start block number for block group 0 */
uint32_t get_inode_table_start_block_nr_for_group_0(void)
{
	/* block group desc table starts at block 1 */
	uint32_t group_desc_sector = raam_root_starting_sector + 8;
	/* read block group desc table for block group 0 */
	char *buf = nvme_read(group_desc_sector, EXT2_BLOCK_SIZE / 512);

	struct ext2_group_desc *desc = (struct ext2_group_desc *) buf;	

	printk("inode_table_start_block = {d}  ", desc->bg_inode_table);

	return desc->bg_inode_table;
}
