#ifndef MEMORY_H                                                               
#define MEMORY_H

#include <boot/boot_params.h>

void pmm_init(struct memory_map_struct memory_map, struct stack_pmm_struct stack_pmm);
static void stack_init(struct stack_pmm_struct *stack_pmm);
static void stack_push(uint64_t page_physical_addr);
static void push_pages_to_stack(uint64_t physical_start_address, uint64_t total_pages);
const int find_total_usable_desc(const int num_desc, char *offset, uint64_t desc_size);

#endif	/* MEMORY_H */
