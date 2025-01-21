#ifndef MAIN_H
#define MAIN_H

#include <boot/boot_params.h>

void main(struct boot_params boot_params);

/* good place to put it for now */
void terminate_process(void);

#endif	// MAIN_H
