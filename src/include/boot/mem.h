#ifndef MEM_H
#define MEM_H

#include <efi.h>
#include <efilib.h>

extern UINTN map_key, memory_map_size, desc_size;
extern char *memory_map;

EFI_STATUS get_memory_map(void);
EFI_STATUS get_memory_map_size(void);
EFI_STATUS allocate_pool_for_mem_map(void);
EFI_STATUS free_pool_for_mem_map(void);
int allocate_sys_variables_mem(void);

#endif	// MEM_H
