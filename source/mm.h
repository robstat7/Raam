#include <efi.h>
#include <efilib.h>
#include <stdint.h>

#define PAGE_SIZE		4096		/* 4k or 4096 bytes */

struct memory_map {
	uint8_t mmap[8900];
	UINTN msize;
	UINTN mkey;
	UINTN dsize;
};
