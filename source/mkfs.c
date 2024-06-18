/*
 * program to make Raam file system on a block device.
 *
 * Raam fs is inspired from MINIX 1.0 FS.
 *
 * See disk_layout.png
 *
 * usage: ./build/mkfs /dev/sda2 num_blocks
 * num_blocks is num_blocks * 1k blocks.
 *
 * zone size = 4k
 * block size = 1k
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

struct super_block {
	int num_nodes;
	int num_zones;
	int num_node_bit_map_blocks;
	int num_zone_bit_map_blocks;
	int first_data_zone;
	int num_zone_blocks;
	long max_file_size; /* in bytes */
	int magic_num; /* in decimal */
};

struct inode {
	char *filename;
	char *dirname;
	int file_size;
	int zone_0;
	int zone_1;
	int zone_2;
	int zone_3;
	int zone_4;
	int zone_5;
	int zone_6;
};

void verify(int fd);

void main(int argc, char *argv[])
{
	int fd;
	struct super_block sb;
	const char *filename;
	int i_node_bit_map[10];
	int *zone_bit_map;
	int i;

	filename =  argv[1];
	sb.num_nodes = 10;
	/* all of the meta information structures will fit in the first 2 zones for now. The rest of the zones are data zones. */
	sb.num_zones = atoi(argv[2])/4 - 2;
	sb.num_node_bit_map_blocks = 1;
	sb.num_zone_bit_map_blocks = 1; /* hard coded for now */
	sb.first_data_zone = 3;
	sb.num_zone_blocks = 4 * sb.num_zones;
	sb.max_file_size = 1024000;
	sb.magic_num = 1237;

	printf("@@@sb.num_zones=%d\n", sb.num_zones);
	printf("@@@sizeof(int)=%ld\n", sizeof(int));
	for (i = 0; i < 10; i++)
		i_node_bit_map[i] = 0;

	 zone_bit_map = malloc(sb.num_zones * sizeof(int));

	for (i=0; i < sb.num_zones; i++)
		zone_bit_map[i] = 0;

	printf("@@@filename = %s\n", filename);
	fd = open(filename, O_RDWR, 0600);
	if (fd == -1) {
		printf("cannot open %s.\n", filename);
		return 1;
	}
	else {
		printf("open %s.\n", filename);
	}

	printf("@@@fd = %d\n", fd);
	printf("@@@sizeof(sb)=%ld\n", sizeof(sb));

	if (write(fd, &sb, sizeof(sb)) == -1)
		printf("could not write SB!\n");
	else
		printf("wrote SB!\n");

	if (lseek(fd, 1024, SEEK_SET) < 1)
		printf("could not seek fd!\n");
	else
		printf("seek fd!\n");

	printf("@@@sizeof(i_node_bit_map)=%ld\n", sizeof(i_node_bit_map));
	if (write(fd, i_node_bit_map, sizeof(i_node_bit_map)) == -1)
		printf("could not write i_node_bit_map!\n");
	else
		printf("wrote i_node_bit_map!\n");

	if (lseek(fd, 2048, SEEK_SET) < 1)
		printf("could not seek fd!\n");
	else
		printf("seek fd!\n");

	if (write(fd, zone_bit_map, sb.num_zones * sizeof(int)) == -1)
		printf("could not write zone_bit_map!\n");
	else
		printf("wrote zone_bit_map!\n");


	free(zone_bit_map);

	if (close(fd) == -1) {
		printf("cannot close %s.\n", filename);
		return 1;
	}
	else {
		printf("close %s.\n", filename);
	}
}

void verify(int fd)
{
	int zone_bit_map[10];

	lseek(fd, 2048, SEEK_SET);
	read(fd, zone_bit_map, sizeof(zone_bit_map));
	for (int i=0; i< 10; i++)
		printf("%d\n", zone_bit_map[i]);
}
