#include <boot/free_stack.h>
#include <boot/mem.h>
#include <boot/boot_params.h>

bool free_stack_init(void)
{
        /* find maximum size. */                                                     
        const uint64_t size = find_free_stack_size();
        if(size == 0) {      /* error */                             
                goto error;                                                      
        }                                                                       

        /* allocate pool for free stack */                                      
        char *base = NULL;                                           
        EFI_STATUS status = allocate_pool_for_free_stack(size, &base);                
        if(EFI_ERROR(status)) {                                                 
                goto error;                                                      
        }                                                                       
                                                                                
        Print(L"@free stack base = %p\n", (void *) base);            
                                                                                
        /* store free stack base and size in the boot params */                 
        boot_params.free_stack.free_stack_base = base;               
        boot_params.free_stack.size = size;

error:
	return false;
}

/*
 * find_free_stack_size
 * --------------------
 * this function finds the free stack (physical memory manager data
 * structure) size by finding the total usable main memory (RAM) pages.
 * The page size is 4 KiB or 4096 bytes.
 *
 * NOTE:
 * potentially, the EFI_MEMORY_DESCRIPTOR structure is only 40
 * bytes despite the UEFI telling me that it should be 48 bytes
 * (the global variable `desc_size`).
 */
uint64_t find_free_stack_size(void)
{
	Print(L"@boot/free_stack.c: getting memory map for the first time!\n");

	EFI_STATUS status = get_memory_map();
	if(EFI_ERROR(status)) {
		goto end;
	}

	Print(L"@boot/free_stack.c: got memory map!\n");

	Print(L"@boot/free_stack.c: memory_map_size = %d\n",
	       memory_map_size);
	Print(L"@boot/free_stack.c: desc_size = %d\n",
	       desc_size);
	Print(L"@boot/free_stack.c: memory map base = %p\n",
	       (void *) memory_map);

	const int num_desc = memory_map_size / desc_size;

	Print(L"@boot/free_stack.c: num_desc = %d\n", num_desc);

	char *offset = memory_map;

	uint64_t total_pages = 0;

	for(int i = 0; i < num_desc; i++) {
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) offset;

		if(desc->Type == EfiBootServicesCode ||
		   desc->Type == EfiBootServicesData ||
		   desc->Type == EfiConventionalMemory) {
			total_pages += (uint64_t) desc->NumberOfPages;
		}

		offset += desc_size;	/* see above NOTE */
	}

	Print(L"@boot/free_stack.c: total usable main mem pages = %p\n",
	       (void *) total_pages);

	return total_pages;

end:
	return 0;
}

/* allocate pool for "free stack" (physical memory manager data structure) */
EFI_STATUS allocate_pool_for_free_stack(const uint64_t total_elements,
					char **free_stack_base_ptr)
{
	EFI_STATUS status;

	/* each stack element is 8 bytes in size */
	status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData,
				   (total_elements * sizeof(uint64_t)),
				   (void **) free_stack_base_ptr);
	if(EFI_ERROR(status)) {
		Print(L"fatal error: error allocating free stack buffer!\n");
		*free_stack_base_ptr = NULL;
	}

	return status;
}
