#ifndef FREE_STACK_H
#define FREE_STACK_H

#include <stdbool.h>
#include <efi.h>
#include <efilib.h>

bool free_stack_init(void);

EFI_STATUS allocate_pool_for_free_stack(const uint64_t total_elements,
					char **free_stack_base_ptr);
uint64_t find_free_stack_size(void);

#endif	/* FREE_STACK_H */
