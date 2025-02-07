#ifndef MEMORY_H                                                               
#define MEMORY_H

#include <boot/boot_params.h>

struct free_stack_pmm_struct {
	uint64_t *base;
	int64_t top;
	uint64_t size;
};

void pmm_init(struct memory_map_struct memory_map,
	      struct free_stack_struct free_stack);
static void free_stack_init(struct free_stack_struct *free_stack);
static void stack_push(uint64_t page_physical_addr);
static void push_pages_to_stack(uint64_t physical_start_address,
				uint64_t total_pages);
const int find_total_usable_uefi_desc(const int num_desc, char *offset,
				      uint64_t desc_size);
void check_stack_contents(void);

#endif	/* MEMORY_H */
