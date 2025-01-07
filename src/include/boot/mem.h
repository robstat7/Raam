#ifndef MEM_H
#define MEM_H

#include <efi.h>
#include <efilib.h>

extern UINTN map_key;

EFI_STATUS get_memory_map(void);
EFI_STATUS get_memory_map_size(void);

#endif	// MEM_H
