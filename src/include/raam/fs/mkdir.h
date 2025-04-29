#ifndef FS_MKDIR_H                                                               
#define FS_MKDIR_H

#include <stdint.h>

/*
 * This is the extent on-disk structure.
 * It's used at the bottom of the tree.
 */
struct ext4_extent {
	uint32_t	ee_block;	/* first logical block extent covers */
	uint16_t	ee_len;		/* number of blocks covered by extent */
	uint16_t	ee_start_hi;	/* high 16 bits of physical block */
	uint32_t	ee_start_lo;	/* low 32 bits of physical block */
};

struct ext4_extent_header {
	uint16_t eh_magic;
	uint16_t eh_entries;	/* number of valid entries */
	uint16_t eh_max;	/* capacity of store in entries */
	uint16_t eh_depth;	/* has tree real underlying blocks? */
	uint32_t eh_generation;	/* generation of the tree */
};

#define EXT4_NAME_LEN 255

/*
 * The new version of the directory entry.  Since EXT4 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext4_dir_entry_2 {
	uint32_t inode;			/* Inode number */
	uint16_t rec_len;		/* Directory entry length */
	uint8_t	name_len;		/* Name length */
	uint8_t	file_type;		/* See file type macros EXT4_FT_* below */
	char	name[EXT4_NAME_LEN];	/* File name */
};

int sys_mkdir(const char *dirname, int mode);
void process_directory_block(uint8_t *block);

#endif	/* FS_MKDIR_H */

