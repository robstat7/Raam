#include <raam/mm/memory.h>
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

	const int total_uefi_desc =
			      memory_map.memory_map_size / memory_map.desc_size;

	// printk("@pmm: total_uefi_desc = {d}  ", total_uefi_desc);

	char *offset = memory_map.memory_map_base;

	/* get the total usable UEFI memory descriptors */
	const int total_usable_uefi_desc =
     find_total_usable_uefi_desc(total_uefi_desc, offset, memory_map.desc_size);

	// printk("@pmm: total usable uefi desc = {d}  ",
	//	  total_usable_uefi_desc);

	/* push USABLE UEFI memory descriptors pages in ascending order to */
	/* the free stack. */
	for(int i = 0; i < total_usable_uefi_desc; i++) {
		uint64_t lowest_physical_start_addr = 0xffffffffffffffff;
							/* uint64_t max value */
		uint64_t total_pages = 0;
		int index = 0;

		offset = memory_map.memory_map_base;

		/* find the lowest physical start address from the list of */
		/* USABLE and UNPUSHED memory descriptors and also the number */
		/* of pages. */
		for(int j = 0; j < total_uefi_desc; j++) {
			EFI_MEMORY_DESCRIPTOR *desc =
					       (EFI_MEMORY_DESCRIPTOR *) offset;

			/* the memory descriptor is of type USABLE and */
			/* its pages have not been pushed to the stack yet. */
			if((desc->Type == EfiBootServicesCode ||
			    desc->Type == EfiBootServicesData ||
			    desc->Type == EfiConventionalMemory)
			    && desc_pushed[j] == false) {
				/* find the lowest phy start address */
				lowest_physical_start_addr =
		((uint64_t) desc->PhysicalStart < lowest_physical_start_addr) ?
		    (uint64_t) desc->PhysicalStart : lowest_physical_start_addr;

				/* get total pages and the index for the UEFI */
				/* memory descriptor. Is the lowest phy start */
				/* address found above the phy start address */
				/* of the current mem descriptor? */
				if(lowest_physical_start_addr ==
					(uint64_t) desc->PhysicalStart) {
					total_pages =
						(uint64_t) desc->NumberOfPages;
					index = j;
				}
			}

			offset += memory_map.desc_size;
		}

		push_pages_to_stack(lowest_physical_start_addr, total_pages);
		
		desc_pushed[index] = true;
	}

	/* store free stack top position to check for */
	/* stack overflow conditions. */
	free_stack_pmm.top_pos = free_stack_pmm.top;

	// printk("@free stack top = {p}  ", (void *) free_stack_pmm.top);
	// printk("@free stack top pos = {p}  ", (void *) free_stack_pmm.top_pos);
	// printk("@free stack size= {p}  ", (void *) free_stack_pmm.size);

	// check_stack_contents();
}

void check_stack_contents(void)
{
	uint64_t i = free_stack_pmm.top - 100;
	// printk("@stack contents:  ");
	for ( ; i < free_stack_pmm.top; i++) {
		// printk("{p}  ", (void *) free_stack_pmm.base[i]);
	}
}

/* find the total USABLE UEFI memory descriptors. */
const int find_total_usable_uefi_desc(const int total_uefi_desc, char *offset,
				      uint64_t desc_size)
{
	int total = 0;

	for(int i = 0; i < total_uefi_desc; i++) {
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

static void push_pages_to_stack(uint64_t physical_start_address,
				const uint64_t total_pages)
{
	for(uint64_t i = 0; i < total_pages; i++) {
		stack_push(physical_start_address);
		physical_start_address += 4096;
				/* move to the next page start address */
	}
}
	

static void free_stack_init(struct free_stack_struct *free_stack)
{
	free_stack_pmm.base = (uint64_t *) free_stack->free_stack_base;	
	free_stack_pmm.top = -1;
	free_stack_pmm.size = free_stack->size;
	/* temperorily set the free stack top position to the stack size. */
	/* Note: this will be helpful when we push USABLE pages to stack */
	/* in pmm_init(). */
	free_stack_pmm.top_pos = free_stack_pmm.size;

	// printk("@pmm: free stack base = {p}  ",
	//	  (void *) free_stack_pmm.base);
}

static void stack_push(uint64_t page_physical_addr)
{
	if(free_stack_pmm.top == free_stack_pmm.top_pos)	{ /* stack overflow */
		printk("error: free stack is full. Can't push {p}!\n",
			(void *) page_physical_addr);
	} else {
		free_stack_pmm.base[++free_stack_pmm.top] = page_physical_addr;
	}
}

/*
 * free_stack_pop
 * --------------
 * This function pops an element from the free stack.
 */
static uint64_t *free_stack_pop(void)
{
	/* first check for free stack underflow condition. */
	if(free_stack_pmm.top == -1) {
		return 0;
	} else {
		return (uint64_t *) free_stack_pmm.base[free_stack_pmm.top--];
	}
}

/*
 * pmm_alloc_page
 * --------------
 * this function allocate one 4096-byte page of physical memory.
 * It returns a pointer that the kernel can use.
 * It returns 0 if the memory cannot be allocated.
 */
uint64_t *pmm_alloc_page(void)
{
	return free_stack_pop();
}

/*
 * pmm_free_page
 * -------------
 * this function frees the page of physical memory pointed at by page_addr,
 * which normally should have been returned by a
 * call to pmm_alloc_page().
 */
void pmm_free_page(uint64_t *page_addr)
{
	if((uint64_t) page_addr % PAGESIZE) {
		printk("panic: pmm_free_page\n");
		for(;;);
	}

	stack_push((uint64_t) page_addr);
}

void test_pmm(void)
{
	uint64_t *page1 = pmm_alloc_page();
	printk("@test_pmm: allocated page addr = {p}  ", (void *) page1);
	uint64_t *page2 = pmm_alloc_page();
	printk("@test_pmm: allocated page addr = {p}  ", (void *) page2);

	pmm_free_page(page1);
	printk("@test_pmm: freed page addr = {p}  ", (void *) page1);
	
	page1 = pmm_alloc_page();
	printk("@test_pmm: allocated page addr = {p}  ", (void *) page1);
	
	uint64_t *page3 = pmm_alloc_page();
	printk("@test_pmm: allocated page addr = {p}  ", (void *) page3);

	
	pmm_free_page(page1);
	printk("@test_pmm: freed page addr = {p}  ", (void *) page1);
	pmm_free_page(page2);
	printk("@test_pmm: freed page addr = {p}  ", (void *) page2);
	pmm_free_page(page3);
	printk("@test_pmm: freed page addr = {p}  ", (void *) page3);
}
