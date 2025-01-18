/*
 * mkfs_ext2
 * ---------
 *
 * make an empty ext2 filesystem on a block device or partition.
 * Using ext 2 revision 0.
 *
 * Notes:
 * - for simplicity, the block size is 1KiB (1024 bytes).
 * - Inode ratio is 16384 (16 KiB or 16384 bytes per inode). This
 *   intermediate value was recommended by chatgpt. Note that inode ratio
 *   specifies the no. of bytes of data that one inode is responsible for.
 * - inode size is 128 fixed (the size of struct).
 * - inode ratio must be >= 1024
 * - reserved blocks ratio should be <= 50
 */
#include <stdio.h>
#include <fs/fs.h>

#define EXT2_MIN_BLOCK_SIZE		1024

/*
 *   - `s_blocks_count`: This value must be lower or equal to
       (s_inodes_per_group * number of block groups). It must be equal
       to the sum of the inodes defined in each block group i.e. including
       the last group (even if it has fewer blocks than BLOCKS_PER_GROUP).
 *
 *   - Calculate the total number of inodes.
 *     INODES = (total_blocks * EXT2_MIN_BLOCK_SIZE) / inode_ratio;
 *
 *     Explanation:
 *        - total_blocks * EXT2_MIN_BLOCK_SIZE: Converts total blocks to total data bytes.
 *        - inode_ratio: Determines how many data bytes each inode represents.
 *        - Ensures there are enough inodes to manage all files and directories.
 *
*/
int main(void)
{
	// int fd = open("/dev/nvme0n1p6", O_RDWR, 0600);
	// printf("opened volume.\n");

	// offset to superblock
	// lseek(fd, 1024, SEEK_SET);

	struct superblock_struct sb;
	

	const int block_size = 1024;	// we will only support this for now
	const int total_blocks = 8195;	// It is the total blocks in the partition 

	sb.s_blocks_per_group = BLOCKS_PER_GROUP;

	/* calculate `s_blocks_count` */

	int full_groups = total_blocks / BLOCKS_PER_GROUP;
	int last_group_blocks = total_blocks % BLOCKS_PER_GROUP;
	sb.s_blocks_count = (full_groups * BLOCKS_PER_GROUP) + last_group_blocks;

	/* calculate s_inodes_count. TODO: verify it using spec */

	const int inode_ratio = 16384;	/* see the starting of file for notes */

	int total_inodes = (total_blocks * EXT2_MIN_BLOCK_SIZE) / inode_ratio;

	sb.s_inodes_count = total_inodes;

	/* calculate reserved blocks count */

	const unsigned reserved_ratio = 5;	// 5% reserved blocks for super user
	uint32_t total_reserved_blocks = (total_blocks * reserved_ratio) / 100;

	sb.s_r_blocks_count = total_reserved_blocks;


	/* calculate total inodes per group */

	
	

	/* print values */
	printf("block size = %d bytes\n", block_size);
	printf("total blocks in the partition = %d\n", total_blocks);
	printf("sb.s_blocks_per_group = %d\n", sb.s_blocks_per_group);
	printf("sb.s_blocks_count = %d\n", sb.s_blocks_count);
	printf("sb.s_inodes_count = %d\n", sb.s_inodes_count);
	printf("sb.s_r_blocks_count = %d\n", sb.s_r_blocks_count);

	return 0;
}	
