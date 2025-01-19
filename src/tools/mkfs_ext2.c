/*
 * mkfs_ext2
 * ---------
 *
 * make a simple empty ext2 filesystem on a volume.
 * Using ext 2 revision 0.
 *
 * Notes:
 * - for simplicity, the block size is 1 KiB (1024 bytes).
 */
#include <stdio.h>
#include <raam/fs.h>

/* normal first block is 1. block 0 contains boot record. */
#define NORMAL_FIRSTBLOCK			1


const int block_size = 1024;	// we will only support this for now

/*
 * main
 * ----
 * notes:
 *   - `s_blocks_count`: This value must be lower or equal to
 *     (s_inodes_per_group * number of block groups). It must be equal
 *     to the sum of the inodes defined in each block group i.e. including
 *     the last group (even if it has fewer blocks than BLOCKS_PER_GROUP).
 *
*/
int main(void)
{
	// int fd = open("/dev/nvme0n1p6", O_RDWR, 0600);
	// printf("opened volume.\n");

	// offset to superblock
	// lseek(fd, 1024, SEEK_SET);	


	const int total_blocks = 256;	// It is the total blocks in the volume 


	struct superblock_struct sb;

	sb.s_blocks_per_group = BLOCKS_PER_GROUP;

	/* calculate `s_blocks_count` */

	int full_groups = total_blocks / BLOCKS_PER_GROUP;
	int last_group_blocks = total_blocks % BLOCKS_PER_GROUP;
	sb.s_blocks_count = (full_groups * BLOCKS_PER_GROUP) + last_group_blocks;

	// /* calculate s_inodes_count. TODO: verify it using spec */

	// const int inode_ratio = 16384;	/* see the starting of file for notes */

	// uint32_t total_inodes = (total_blocks * EXT2_MIN_BLOCK_SIZE) / inode_ratio;


	// /* calculate reserved blocks count */

	// const unsigned reserved_ratio = 5;	// 5% reserved blocks for super user
	// uint32_t total_reserved_blocks = (total_blocks * reserved_ratio) / 100;

	sb.s_r_blocks_count = 0;	/* for simplicity, we won't use it now */


	// /* calculate total inodes per group */

	// const int frags_per_block = block_size / frag_size; /* fragments per block */

	// /* divide the total usable blocks by blocks per group. Block 0 is not usable */
	// const unsigned long group_desc_count = (((total_blocks - NORM_FIRSTBLOCK) * frags_per_block) / BLOCKS_PER_GROUP);	/* number of group descriptors */

	// int inodes_per_group = 0;
	// /* round the inodes per group count to fully use each group descriptor */
        // if (sb.s_inodes_count % group_desc_count)                                          
        //         inodes_per_group = (sb.s_inodes_count / group_desc_count) + 1;               
        //  else                                                                   
        //         inodes_per_group = sb.s_inodes_count / group_desc_count;               

	// /* Round the inodes pet group count to fully use each block in each descriptor */         
        // if (inodes_per_group % EXT2_INODES_PER_BLOCK)                             
        //         inodes_per_group = ((inodes_per_group / EXT2_INODES_PER_BLOCK) + 1) *
        //                            EXT2_INODES_PER_BLOCK;                       
        // total_inodes = inodes_per_group * group_desc_count;

	// 
	// sb.s_inodes_count = total_inodes;
	// sb.s_inodes_per_group = inodes_per_group;	

	// /* id of the block containing the superblock structure. */
	// sb.s_first_data_block = NORM_FIRSTBLOCK;

	// sb.s_log_block_size = EXT4_LOG_BLOCK_SIZE;	/* 1024 = 1024 << 0 */

	// sb.s_log_frag_size = EXT4_LOG_FRAG_SIZE;	/* 0 for 1024 */

	// uint32_t frags_per_group = BLOCKS_PER_GROUP * (block_size / frag_size);

	// sb.s_frags_per_group = frags_per_group;

	// sb.s_mtime = 0;

	// sb.s_wtime = time(NULL);

	/* print values */
	printf("block size = %d bytes\n", block_size);
	printf("total blocks in the partition = %d\n", total_blocks);
	printf("sb.s_blocks_per_group = %d\n", sb.s_blocks_per_group);
	printf("sb.s_blocks_count = %d\n", sb.s_blocks_count);
	// printf("sb.s_inodes_count = %d\n", sb.s_inodes_count);
	printf("sb.s_r_blocks_count = %d\n", sb.s_r_blocks_count);
	// printf("sb.s_inodes_per_group = %d\n", sb.s_inodes_per_group);
	// printf("sb.s_wtime = %d\n", sb.s_wtime);

	return 0;
}	
