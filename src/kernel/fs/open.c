#include <lib/string.h>
#include <raam/printk.h>
#include <raam/fs/open.h>

int sys_open(const char *filename, int flag, int mode)
{
	char only_file_name[100];

	get_only_file_name(filename, only_file_name);

	printk("only_file_name: ");
	printk(only_file_name);

	return 1;
}

static void get_only_file_name(const char *filename, const char *only_file_name)
{
	strncpy(only_file_name, filename + 1, strlen(filename));
}

