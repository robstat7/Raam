#ifndef MEM_H
#define MEM_H

#include <efi.h>
#include <efilib.h>

extern UINTN map_key;

EFI_STATUS get_memory_map(void);
EFI_STATUS get_memory_map_size(void);
EFI_STATUS allocate_pool_for_mem_map(void);
EFI_STATUS free_pool_for_mem_map(void);
int allocate_sys_variables_mem(void);
EFI_STATUS allocate_pool_for_stack_pmm(const uint64_t total_pages, char **stack);
uint64_t find_num_usable_main_memory_4kib_pages(void);

#endif	// MEM_H
