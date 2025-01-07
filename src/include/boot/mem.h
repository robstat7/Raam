#ifndef MEM_H
#define MEM_H

#include <efi.h>
#include <efilib.h>

extern UINTN map_key;

EFI_STATUS get_memory_map(void);
EFI_STATUS get_memory_map_size(void);
EFI_STATUS allocate_pool_for_mem_map(void);

#endif	// MEM_H
