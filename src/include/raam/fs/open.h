#ifndef OPEN_H
#define OPEN_H

int sys_open(const char *filename, int flag, int mode);
static void get_only_file_name(const char *filename, const char *only_file_name);

#endif	/* OPEN_H */
