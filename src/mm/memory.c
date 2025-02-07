#include <mm/memory.h>
#include <stdbool.h>
#include <raam/printk.h>

struct free_stack_pmm_struct free_stack_pmm;

/*
 * pmm_init
 * --------
 * physical memory manager initialization. This function initializes the
 * free stack, gets the total usable UEFI memory descriptors, and then
 * pushes the pages of each usable descriptor to the free stack. The
 * free stack must contain the lowest page start address at the bottom
 * and the highest page start address at the top.
 *
 * note:
 *    the page size is 4096 bytes.
 */
void pmm_init(struct memory_map_struct memory_map,
	      struct free_stack_struct free_stack)
{
	free_stack_init(&free_stack);

	/* tells us if a UEFI memory descriptor is */
	/* pushed or unpushed into the stack. */
	bool desc_pushed[500] = {false, };

	const int num_desc = memory_map.memory_map_size / memory_map.desc_size;

	printk("@pmm: num_desc = {d}  ", num_desc);

	char *offset = memory_map.memory_map_base;

	/* get total usable UEFI memory descriptors */
	const int total_usable_uefi_desc =
	    find_total_usable_uefi_desc(num_desc, offset, memory_map.desc_size);
	printk("@pmm: total usable uefi desc = {d}  ", total_usable_uefi_desc);

	for(int i = 0; i < total_usable_uefi_desc; i++) {
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

	printk("@stack top = {p}  ", (void *) free_stack_pmm.top);
	printk("@stack size= {p}  ", (void *) free_stack_pmm.size);

	// check_stack_contents();
}

void check_stack_contents(void)
{
	uint64_t i = 0;
	printk("@stack contents:  ");
	for ( ; i < 100; i++) {
		printk("{p}  ", (void *) free_stack_pmm.base[i]);
	}
}

/* find the total USABLE UEFI memory descriptors. */
const int find_total_usable_uefi_desc(const int num_desc, char *offset,
				      uint64_t desc_size)
{
	int total = 0;

	for(int i = 0; i < num_desc; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) offset;

		/* the UEFI memory descriptor is of type USABLE */
		if(desc->Type == EfiBootServicesCode ||
		   desc->Type == EfiBootServicesData ||
		   desc->Type == EfiConventionalMemory) {
			total++;			
		}
	
		offset += desc_size;
	}

	return total;
}

static void push_pages_to_stack(uint64_t physical_start_address, uint64_t total_pages)
{
	for(uint64_t i = 0; i < total_pages; i++) {
		stack_push(physical_start_address);
		physical_start_address += 4096;	/* move to next page start address */
	}
}
	

static void free_stack_init(struct free_stack_struct *free_stack)
{
	free_stack_pmm.base = (uint64_t *) free_stack->free_stack_base;	
	free_stack_pmm.top = -1;
	free_stack_pmm.size = free_stack->size;

	printk("@pmm: free stack base = {p}  ", (void *) free_stack_pmm.base);
}

static void stack_push(uint64_t page_physical_addr)
{
	free_stack_pmm.top += 1;

	free_stack_pmm.base[free_stack_pmm.top] = page_physical_addr;
}	
