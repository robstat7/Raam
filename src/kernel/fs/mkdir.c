#include <lib/string.h>
#include <raam/printk.h>
#include <raam/fs/mkdir.h>
#include <raam/nvme.h>

#define EXT4_EXTENTS_FL         0x00080000  /* File uses extents */
#define EXT4_INLINE_DATA_FL     0x10000000 // File has inline data

#define EXT4_FT_DIR		2

#define S_IFDIR			0x4000	/* directory */

#define NVME_LOGICAL_BLOCK_SIZE		512	/* in bytes */
#define EXT4_BLOCK_SIZE			4096	/* in bytes */

const long raam_root_first_sector = 997109760;

int find_first_unset_bit(uint16_t bitmap);
void write_inode_bitmap(char *bitmap_ptr, uint16_t value);
void write_directory_block(uint8_t *block, int new_inode, char *new_dir_name);
int allocate_block(uint8_t *bitmap, int size);
void write_new_dir_data_block(uint8_t * block, uint32_t inode);
void write_dir_entry(struct ext4_dir_entry_2 *entry, uint32_t inode, char *name, uint8_t file_type);

int sys_mkdir(const char *dirname, int mode)
{
	char dir_name_only[10];
	uint64_t nvme_sector_for_root_data_block = -1;

	/* find path in FS */
	char dir_path[10];

	strncpy(dir_path, dirname, 1);

	dir_path[1] = '\0';

	printk("dir_path: ");
	printk(dir_path);
	printk("  ");

	/* find only directory name */
	int len = strlen(dirname);
	strncpy(dir_name_only, dirname + 1, len);

	printk("dir_name_only: ");
	printk(dir_name_only);
	printk("  ");


	/* read root's inode */

	/* read superblock for Raam root partition */
	/* NOTE: ext4 block size = 4096, nvme logical block size = 512 bytes */	
	char *buf = nvme_read(raam_root_first_sector + 2, 2); /* superblock size is 1024 bytes */

	const uint16_t s_inode_size = *((uint16_t *) (buf + 0x58));

	printk("s_inode_size = {d}  ", s_inode_size);

	/* read group desc for group 0 starting at block 1. the desc size is 32 bytes. */
	buf = nvme_read(raam_root_first_sector + 8, 1);

	uint32_t bg_inode_table_lo = *((uint32_t *) (buf + 0x8)); /* a block nr. */
	printk("bg_inode_table_lo = {d}  ", bg_inode_table_lo);


	const long inode_table_grp_0_starting_sector = raam_root_first_sector + (bg_inode_table_lo * 8);

	buf = nvme_read(inode_table_grp_0_starting_sector, 1);

	const char *root_inode = (buf + s_inode_size); /* inode nr. 2 */

	/* check it is actually a directory */
	uint16_t i_mode = *((uint16_t *) root_inode);

	printk("{p}  ", i_mode & 0xf000);	/* 0x4000 means inode is a directory */

	/* read root's data */

	/* check if the directory uses extents or traditional block pointers */
	uint32_t i_flags = *((uint32_t *) (root_inode + 0x20));

	/* file uses extents */
	if (i_flags & EXT4_EXTENTS_FL) {
		const char *i_block = root_inode + 0x28;
		struct ext4_extent_header *eh = (struct ext4_extent_header *) i_block;	

		printk("eh_magic = {p}  ", eh->eh_magic);
		printk("eh_depth = {d}  ", eh->eh_depth);

		if (eh->eh_depth == 0) {
        		// Extent entries are inline in the inode
			struct ext4_extent *extents = (struct ext4_extent *) (eh + 1);
			uint64_t physical_block = ((uint64_t) extents[0].ee_start_hi << 32) | extents[0].ee_start_lo;	/* a block nr. in the FS */
			uint32_t length = extents[0].ee_len;

			 printk("Extent 0: Logical start={d}, Physical start={p}, Length={d} blocks  ", extents[0].ee_block, physical_block, length);

			/* get sector to read the root dir data block */
			nvme_sector_for_root_data_block = raam_root_first_sector + (physical_block * (EXT4_BLOCK_SIZE / NVME_LOGICAL_BLOCK_SIZE));

			// Read 8 sectors (4096 bytes) via NVMe
			buf = nvme_read((uint32_t) nvme_sector_for_root_data_block, 8);

			// process_directory_block(buf);
		}
	}

	/* write inode bitmap for inode 12 (second inode) */

	/* read group desc for group 0 starting at block 1. the desc size is 32 bytes. */
	buf = nvme_read(raam_root_first_sector + 8, 1);

	uint32_t bg_inode_bitmap_lo = *((uint32_t *) (buf + 0x4)); /* a block nr. */
	printk("bg_inode_bitmap_lo = {d}  ", bg_inode_bitmap_lo);

	uint32_t nvme_sector_for_inode_bitmap = raam_root_first_sector + (bg_inode_bitmap_lo * (EXT4_BLOCK_SIZE / NVME_LOGICAL_BLOCK_SIZE));

	buf = nvme_read(nvme_sector_for_inode_bitmap, 1);	

	uint16_t bitmap_val = *((uint16_t *) buf);

	printk("bitmap_val = {d}  ", bitmap_val);

	// Find first unset bit
    	int bit_pos = find_first_unset_bit(bitmap_val);

	// Set the bit
    	bitmap_val |= (1 << bit_pos);

	// Write back to disk
    	write_inode_bitmap(buf, bitmap_val);
    	nvme_write(nvme_sector_for_inode_bitmap, 1); // Write updated sector

	printk("bit_pos = {d}  ", bit_pos);
	printk("new bitmap val = {d}  ", bitmap_val);

	int new_inode_nr = bit_pos + 1;	/* inode nr. for new dir */


	/* root dir's data write */

	buf = nvme_read((uint32_t) nvme_sector_for_root_data_block, 8);

	write_directory_block(buf, new_inode_nr, dir_name_only);

	nvme_write((uint32_t) nvme_sector_for_root_data_block, 8);


	/* allocate a new data block for the new inode in group 0 */

	
	/* read group desc for group 0 starting at block 1. the desc size is 32 bytes. */
	buf = nvme_read(raam_root_first_sector + 8, 1);

	uint32_t bg_block_bitmap_lo = *((uint32_t *) buf); /* a block nr. */
	printk("bg_block_bitmap_lo = {d}  ", bg_block_bitmap_lo);

	uint32_t nvme_sector_for_block_bitmap = raam_root_first_sector + (bg_block_bitmap_lo * (EXT4_BLOCK_SIZE / NVME_LOGICAL_BLOCK_SIZE));

	buf = nvme_read(nvme_sector_for_block_bitmap, 2);	

	const int new_data_block = allocate_block((uint8_t *) buf, 512 * 2);
		
	printk("new_data_block = {d}  ", new_data_block);

	// Write block bitmap data back to disk
    	nvme_write(nvme_sector_for_block_bitmap, 2); 


	/* write new inode data block */

	uint32_t nvme_sector_for_new_data_block = raam_root_first_sector + (new_data_block * (EXT4_BLOCK_SIZE / NVME_LOGICAL_BLOCK_SIZE));
	buf = nvme_read(nvme_sector_for_new_data_block, 8);	
	write_new_dir_data_block((uint8_t *) buf, new_inode_nr);

	nvme_write(nvme_sector_for_new_data_block, 8);


	/* write new directory's inode */


	buf = nvme_read(inode_table_grp_0_starting_sector, 6);

	const char *new_inode = buf + ((new_inode_nr - 1) * s_inode_size);

	uint16_t * new_inode_mode =  (uint16_t *) new_inode;  /* at offset 0 */

	*new_inode_mode = S_IFDIR | 0755;	

	/* set i_links_count to 1 as DIR_NLINK feature is enabled */
	uint16_t *i_links_count = (uint16_t *) (new_inode + 0x1a);

	*i_links_count = 1;

	uint32_t *i_size_lo = (uint32_t *) (new_inode + 0x4);
	*i_size_lo = EXT4_BLOCK_SIZE;	// 1 block

	uint32_t *new_i_flags =(uint32_t *) (new_inode + 0x20); 

	*new_i_flags = EXT4_EXTENTS_FL;

	/* create extent tree */
	const char *new_inode_i_block = new_inode + 0x28;
	struct ext4_extent_header *eh = (struct ext4_extent_header *) new_inode_i_block;	
	eh->eh_magic = 0xf30a;
	eh->eh_entries = 1;	/* siince there's one extent entry */
	eh->eh_max = 4;		/* Max 4 entries fit in i_block */
	eh->eh_depth = 0;	/* Leaf node (no tree depth) */
	eh->eh_generation = 0;	/* Optional: generation number */

	struct ext4_extent *extents = (struct ext4_extent *) (eh + 1);

	extents[0].ee_block = 0;	/* Logical block 0 (first block) */
	extents[0].ee_len = 1;		/* 1 block allocated */
	extents[0].ee_start_hi = new_data_block >> 32;
	extents[0].ee_start_lo = new_data_block & 0xffffffff;

	nvme_write(inode_table_grp_0_starting_sector, 6);

	return 0;
}

// Write back to disk
void write_inode_bitmap(char *bitmap_ptr, uint16_t value) {
    bitmap_ptr[0] = (value & 0xff);        // Low byte
    bitmap_ptr[1] = ((value >> 8) & 0xff); // High byte
}

// Find the first unset bit (0) in a 16-bit value
int find_first_unset_bit(uint16_t bitmap) {
    if (bitmap == 0xffff) return -1; // All bits set
    uint16_t inverted = ~bitmap;
    return __builtin_ffs(inverted) - 1; // ffs returns 1-based index
}

void process_directory_block(uint8_t *block)
{
	uint8_t *ptr = block;
    	uint8_t *end = block + 4096;

	int i = 1;

	while (ptr < end) {
        	struct ext4_dir_entry_2 *entry = (struct ext4_dir_entry_2 *)ptr;

        	// // Skip deleted entries
        	// if (entry->inode == 0) {
		//     i++;
        	//     ptr += entry->rec_len;
        	//     continue;
        	// }

        	// Extract name (ensure null termination)
        	char name[256];
        	strncpy(name, entry->name, entry->name_len);
        	name[entry->name_len] = '\0';

        	printk("Directory entry: entry_no.={d}, inode={d}, type={d}, len={d}, name=",
        	       i, entry->inode, entry->file_type, entry->rec_len);

		printk(name);

		printk("  ");

		i++;

        	ptr += entry->rec_len;
    }
}

void write_directory_block(uint8_t *block, int new_inode, char *new_dir_name)
{
	uint8_t *ptr = block;
    	uint8_t *end = block + 4096;

	while (ptr < end) {
        	struct ext4_dir_entry_2 *entry = (struct ext4_dir_entry_2 *)ptr;

		/* use deleted entry */
        	if (entry->inode == 0) {
			entry->inode = new_inode;
			entry->name_len = strlen(new_dir_name);
			entry->file_type = EXT4_FT_DIR;
			strncpy(entry->name, new_dir_name, entry->name_len);
			break;
		}
		
        	ptr += entry->rec_len;
	}

}

/**
 * Find the first unset (0) bit in the bitmap and set it.
 * Returns: the index of the allocated block (0-based), or -1 if full.
 */
int allocate_block(uint8_t *bitmap, int size)
{
	for (int byte = 0; byte < size; byte++) {
		if (bitmap[byte] != 0xff) { // Only check bytes with at least 1 free bit
			for (int bit = 0; bit < 8; bit++) {
				if (!(bitmap[byte] & (1 << bit))) {
					bitmap[byte] |= (1 << bit); // set the bit
					return byte * 8 + bit;      // return block index
				}
			}
		}
	}
	return -1; // no free blocks
}

void write_new_dir_data_block(uint8_t * block, uint32_t inode)
{
	uint8_t *ptr = block;

	struct ext4_dir_entry_2 *entry = (struct ext4_dir_entry_2 *)ptr;
	write_dir_entry(entry, inode, ".", EXT4_FT_DIR);

        ptr += entry->rec_len;

	entry = (struct ext4_dir_entry_2 *) ptr;
	write_dir_entry(entry, 2, "..", EXT4_FT_DIR);

	entry->rec_len = EXT4_BLOCK_SIZE - entry->rec_len;	/* last entry should fill the rest of the block */
}

void write_dir_entry(struct ext4_dir_entry_2 *entry, uint32_t inode, char *name, uint8_t file_type)
{
	entry->inode = inode;
	entry->name_len = strlen(name);
	entry->file_type = file_type;
	strncpy(entry->name, name, entry->name_len);

	uint16_t len = 8 + entry->name_len;	/* 8 is the size of inode, rec_len, name_len, and file_type */

	entry->rec_len = len + (len % 4);	/* dir entry should be 4-byte aligned */
}
