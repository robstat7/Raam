/*
 * Write the text file 'test.txt' to the file system.
 * We will later open it from our kernel using
 * the open syscall.
 */
#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void verify(int fd);

void main(int argc, char *argv[])
{
	struct inode node;
	int fd_usb, fd_test;
	char *filename;
	char test_file[29];
	int bit = 1;

	filename = argv[1];
	node.filename = "test.txt";
	node.dirname = "/";
	node.file_size = 29; /* in bytes */
	node.zone_0 = 3;
	node.zone_1 = -1;

	printf("@@@sizeof(node)=%ld\n", sizeof(node));
	fd_usb = open(filename, O_RDWR, 0600);
	if (fd_usb == -1) {
		printf("cannot open %s.\n", filename);
		return 1;
	}

	fd_test = open(node.filename, O_RDONLY, 0600);
	if (fd_test == -1) {
		printf("cannot open %s.\n", node.filename);
		return 1;
	}                                             	

	lseek(fd_usb, 3072, SEEK_SET);

	if (write(fd_usb, &node, sizeof(node)) == -1)
		printf("could not write node!\n");
	else
		printf("wrote node!\n");

	read(fd_test, test_file, 29);

	lseek(fd_usb, 8192, SEEK_SET);

	if (write(fd_usb, test_file, sizeof(test_file)) == -1)
		printf("could not write file!\n");
	else
		printf("wrote file!\n");

	/* set node bit map bit 1 to 1 */
	lseek(fd_usb, 1024, SEEK_SET);

	write(fd_usb, &bit, sizeof(bit));

	/* set zone bit map bit 1 to 1 */
	lseek(fd_usb, 2048, SEEK_SET);

	write(fd_usb, &bit, sizeof(bit));

	verify(fd_usb);

	if (close(fd_usb) == -1) {
		printf("cannot close %s.\n", filename);
		return 1;
	}
	else {
		printf("close %s.\n", node.filename);
	}
	close(fd_test);
}

void verify(int fd_usb)
{
	struct inode node;

	lseek(fd_usb, 3072, SEEK_SET);

	read(fd_usb, &node, sizeof(node));
	printf("%d\n", node.file_size);

}
