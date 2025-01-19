/*
 * mkfs_raam.c
 * -----------
 * make Raam filesystem on a volume.
 */
#include <raam/fs.h>
#include <stdio.h>
#include <fcntl.h>                                       
#include <unistd.h>
#include <stdlib.h>

const int total_data_blocks = 10;	/* fixed */
const int total_inodes = 10;	/* fixed */

void set_block_used(int block_num, uint8_t *block_bitmap_block);

int main(void)
{
	struct superblock_struct super;

	super.magic = MAGIC;

	super.block_size = BLOCK_SIZE;

	super.total_blocks = total_data_blocks + 4;

	super.total_data_blocks = total_data_blocks;

	super.total_inodes = total_inodes;

	super.first_data_block = 4; /* 0 to 3 block nr. contains metadata */

	/* create zeroed block bitmap block */
	uint8_t *block_bitmap_block = (char *) malloc(BLOCK_SIZE);
	if(block_bitmap_block == NULL) {
		printf("error: could not allocate block bitmap block!\n");
		goto end_4;
	}	

	memset(block_bitmap_block, 0, BLOCK_SIZE);	/* fill block #1 with 0 */

	/* block #0, #1, #2, and #3 are already used by metadata. Set bits
	   accordingly. */
	for(int i = 0; i < 4; i++) {
		set_block_used(i, block_bitmap_block);	
	}

	/* create zeroed inode bitmap block */
	uint8_t *inode_bitmap_block = (char *) malloc(BLOCK_SIZE);
	if(inode_bitmap_block == NULL) {
		printf("error: could not allocate inode bitmap block!\n");
		goto end_3;
	}	

	memset(inode_bitmap_block, 0, BLOCK_SIZE);	/* fill block #2 with 0 */



	/* print values */
	printf("super.magic = %p\n", (void *) super.magic);
	printf("super.block_size = %d bytes\n", super.block_size);
	printf("super.total_blocks = %d\n", super.total_blocks);
	printf("super.total_data_blocks = %d\n", super.total_data_blocks);
	printf("super.total_inodes = %d\n", super.total_inodes);
	printf("super.first_data_block = %d\n", super.first_data_block);

	/* write the file system layout to the volume */                
        int fd = open("/dev/nvme0n1p6", O_RDWR, 0600);             
        if(fd == -1) {                                                         
                printf("cannot open volume.\n");                             
		goto end_2;
        } else {                                                          
                printf("opened volume.\n");                                  
        }                         

	if(write(fd, &super, sizeof(super)) == -1) {
		printf("could not write superblock!\n");
		goto end_1;
	} else {
		printf("wrote superblock!\n");
	}

	if(lseek(fd, 512, SEEK_SET) < 1) {
		printf("could not seek fd!\n");
		goto end_1;
	} else {
		printf("seeked fd!\n");
	}

	if(write(fd, block_bitmap_block, BLOCK_SIZE) == -1) {
		printf("could not write block bitmap block!\n");
		goto end_1;
	} else {
		printf("wrote block bitmap block!\n");
	}

	if(lseek(fd, 1024, SEEK_SET) < 1) {
		printf("could not seek fd!\n");
		goto end_1;
	} else {
		printf("seeked fd!\n");
	}

	if(write(fd, inode_bitmap_block, BLOCK_SIZE) == -1) {
		printf("could not write inode bitmap block!\n");
		goto end_1;
	} else {
		printf("wrote inode bitmap block!\n");
	}


end_1:
	close(fd);
end_2:
	free(block_bitmap_block);
end_3:
	free(inode_bitmap_block);
end_4:
	return 0;
}

void set_block_used(int block_num, uint8_t *block_bitmap_block) {
    // Determine which byte and bit within the byte corresponds to the block
    int byte_index = block_num / 8;     // Index of the byte in the bitmap
    int bit_index = block_num % 8;      // Bit position within the byte

    // Set the bit corresponding to the block to 1 (used)
    block_bitmap_block[byte_index] |= (1 << bit_index);  // Use bitwise OR to set the bit
}
