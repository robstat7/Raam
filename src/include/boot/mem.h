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
EFI_STATUS allocate_pool_for_free_stack(const uint64_t total_elements,
					char **free_stack_base_ptr);
uint64_t find_free_stack_size(void);

#endif	// MEM_H
