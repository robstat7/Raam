#include <mm/memory.h>
#include <stdbool.h>
#include <raam/printk.h>

uint64_t *stack = NULL;
int64_t stack_top = -1;
uint64_t stack_size = 0;

/*
 * physical memory manager initialization.
 */
void pmm_init(struct memory_map_struct memory_map, struct stack_pmm_struct stack_pmm)
{
	stack_init(&stack_pmm);

	bool desc_pushed[500] = {false, };
					/* tells us if a memory descriptor is */
					/* pushed or unpushed into the stack. */

	const int num_desc = memory_map.memory_map_size / memory_map.desc_size;

	printk("@pmm: num_desc = {d}  ", num_desc);

	char *offset = memory_map.memory_map_base;

	/* get total usable memory descriptors */
	const int total_usable_desc = find_total_usable_desc(num_desc, offset, memory_map.desc_size);
	printk("@pmm: total usable desc = {d}  ", total_usable_desc);

	for(int i = 0; i < total_usable_desc; i++) {
		uint64_t lowest_physical_start_addr = 0xffffffffffffffff;	/* uint64_t max value */
		uint64_t total_pages = 0;
		int index = 0;

		offset = memory_map.memory_map_base;

		/* find the lowest physical start address of a usable and upushed memory descriptor and also its number of pages */
		for(int j = 0; j < num_desc; j++) {
			EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) offset;

			/* the memory descriptor is of type USABLE and its pages have not been pushed to the stack yet. */
			if((desc->Type == EfiBootServicesCode || desc->Type == EfiBootServicesData || desc->Type == EfiConventionalMemory) && desc_pushed[j] == false) {
				lowest_physical_start_addr  = ((uint64_t) desc->PhysicalStart < lowest_physical_start_addr) ?
							(uint64_t) desc->PhysicalStart : lowest_physical_start_addr;
				if(lowest_physical_start_addr == (uint64_t) desc->PhysicalStart) {
					total_pages = (uint64_t) desc->NumberOfPages;
					index = j;
				}
			}

			offset += memory_map.desc_size;
		}

		// printk("@debug before push!!!  ");

		push_pages_to_stack(lowest_physical_start_addr, total_pages);
		
		// printk("@debug after push!!!  ");

		desc_pushed[index] = true;
	}

	// printk("@debug!!!  ");

	/* check */ 
	if(stack_top == stack_size - 1) {
		printk("stack is full!!!  ");
		printk("stack top = {p}  ", (void *) stack_top);
	} else {
		printk("pmm: error: stack is not full!!!  ");
		printk("stack top = {p}  ", (void *) stack_top);
		printk("stack size= {p}  ", (void *) stack_size);
	}
}

const int find_total_usable_desc(const int num_desc, char *offset, uint64_t desc_size)
{
	int total = 0;

	for(int i = 0; i < num_desc; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) offset;

		/* the memory descriptor is of type USABLE */
		if(desc->Type == EfiBootServicesCode || desc->Type == EfiBootServicesData || desc->Type == EfiConventionalMemory) {
			total++;			
		}
	
		offset += desc_size;
	}

	return total;
}



// static void find_lowest_physical_start_addr(uint64_t *lowest_physical_start_addr, uint64_t physical_start_addr)
// {
// 	*lowest_physical_start_addr = (physical_start_addr < *lowest_physical_start_addr) ?
// 					physical_start_addr : *lowest_physical_start_addr;
// }

static void push_pages_to_stack(uint64_t physical_start_address, uint64_t total_pages)
{
	for(uint64_t i = 0; i < total_pages; i++) {
		stack_push(physical_start_address);
		physical_start_address += 4096;	/* move to next page start address */
	}
}
	

static void stack_init(struct stack_pmm_struct *stack_pmm)
{
	stack = (uint64_t *) stack_pmm->stack;	
	stack_top = -1;
	stack_size = stack_pmm->total_pages;

	printk("@pmm: stack base = {p}  ", (void *) stack);
}

static void stack_push(uint64_t page_physical_addr)
{
	stack_top += 1;

	stack[stack_top] = page_physical_addr;
}	
