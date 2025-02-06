/*
 * boot time memory related functions.
 */
#include <boot/mem.h>
#include <boot/boot_params.h>

UINTN memory_map_size = 0, desc_size = 0;
UINTN map_key = 0;
char *memory_map = NULL;

/*
 * get_memory_map
 *
 * This function gets the memory map size, updates it, and allocates a new
 * pool of memory for the memory map. Then it gets the memory map into
 * this newly allocated pool.
 *
 * NOTE: allocating the pool creates at least one new descriptor for
 * the chunk of memory changed to EfiLoaderData. Not sure that UEFI
 * firmware must allocate on a memory type boundary! If not, then two
 * descriptors might be created.
 * We will use this updated memory map size to allocate pool for the
 * memory map and to get the memory map later.
 */
EFI_STATUS get_memory_map(void)
{
	EFI_STATUS status;

	memory_map_size = 0;
	desc_size = 0;
	map_key = 0;
	memory_map = NULL;

	status = get_memory_map_size();
	if(status != EFI_BUFFER_TOO_SMALL) {
		goto end;
	}

	// updating memory map size. See the NOTE above
	memory_map_size += 2 * desc_size;

	status = allocate_pool_for_mem_map();
	if(EFI_ERROR(status)) {
		goto end;
	}

	status = uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size,
				   memory_map, &map_key, &desc_size, NULL);
	if(EFI_ERROR(status)) {
		Print(L"fatal error: error getting memory map!\n");
	}

	// /* store memory map params to the boot params */
	// boot_params.memory_map.memory_map_size = memory_map_size;	
	// boot_params.memory_map.desc_size = desc_size;	
	// boot_params.memory_map.memory_map_base = memory_map;	
end:
	return status;
}

/*
 * allocate_pool_for_mem_map
 *
 * This function invokes BootServices->AllocatePool function and
 * allocates a new pool of memory for the memory map.
 */
EFI_STATUS allocate_pool_for_mem_map(void)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData,
				   memory_map_size, (void **) &memory_map);
	if(EFI_ERROR(status)) {
		Print(L"fatal error: error allocating memory map buffer!\n");
		memory_map = NULL;
	}

	return status;
}

EFI_STATUS free_pool_for_mem_map(void)
{
	EFI_STATUS status = uefi_call_wrapper(BS->FreePool, 1,
					      (void *)  memory_map);
	if(EFI_ERROR(status)) {
		Print(L"fatal error: error freeing memory map buffer!\n");
	}

	return status;
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


/*
 * get_memory_map_size
 *
 * This function gets the memory map size.
 */
EFI_STATUS get_memory_map_size(void)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->GetMemoryMap,5,&memory_map_size,0,
				   &map_key,&desc_size,NULL);	
	if(status != EFI_BUFFER_TOO_SMALL) {
		Print(L"fatal error: error getting memory map size!\n");
		memory_map_size = 0;
	}

	return status;
}

/**
 * allocate_sys_variables_mem
 * ---------------------------
 * allocates and clears 2 MiB of memory for system variables. A system
 * variable is a variable used by the kernel to store important
 * configuration data or state information that is critical for system
 * operation and management. Example use case is in the NVMe driver.
 * It also stores the pointer in the boot params.
 *
 * @param void
 * @return 0 on success, -1 on failure.
 */
int allocate_sys_variables_mem(void)
{
	uint8_t **sys_var_ptr_ptr;

	const UINTN MEMORY_SIZE = 2 * 1024 * 1024; // 2 MiB
	// Memory type for allocation
	const EFI_MEMORY_TYPE MEMORY_TYPE = EfiLoaderData;

	// allocate 2 MiB of memory
	EFI_STATUS status = uefi_call_wrapper(BS->AllocatePool, 3, MEMORY_TYPE,
					MEMORY_SIZE, (void **) sys_var_ptr_ptr);
	if (EFI_ERROR(status)) {
	    Print(L"error: allocate_pool failed with status %x!\n", status);
	    *sys_var_ptr_ptr = NULL; // ensure the pointer is null on failure
	    return -1;
	}
	
	// clear the allocated memory by setting it to 0
	uefi_call_wrapper(BS->SetMem, 3, (void *) *sys_var_ptr_ptr,
			  MEMORY_SIZE, 0);

	/* store the sys var pointer in the boot params */
	boot_params.system_variables = *sys_var_ptr_ptr;	
	
	return 0;
}

/*
 * find_total_usable_main_memory_4kib_pages
 * ----------------------------------------
 * this function finds the total usable main memory (RAM) pages that we
 * can use for physical memory allocation. The page size is 4 KiB or
 * 4096 bytes.
 *
 * NOTE:
 * potentially, the EFI_MEMORY_DESCRIPTOR structure is only 40
 * bytes despite the UEFI telling me that it should be 48 bytes
 * (the global variable `desc_size`).
 */
uint64_t find_total_usable_main_memory_4kib_pages(void)
{
	Print(L"@boot/mem.c: getting memory map for the first time!\n");

	EFI_STATUS status = get_memory_map();
	if(EFI_ERROR(status)) {
		goto end;
	}

	/* store memory map params to the boot params */
	boot_params.memory_map.memory_map_size = memory_map_size;	
	boot_params.memory_map.desc_size = desc_size;	
	boot_params.memory_map.memory_map_base = memory_map;	
	
	Print(L"@boot/mem.c: got memory map!\n");

	Print(L"@boot/mem.c: memory_map_size = %d\n",
	       memory_map_size);
	Print(L"@boot/mem.c: desc_size = %d\n",
	       desc_size);
	Print(L"@boot/mem.c: memory map base = %p\n",
	       (void *) memory_map);

	const int num_desc = memory_map_size / desc_size;

	Print(L"@boot/mem.c: num_desc = %d\n", num_desc);

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

	Print(L"@boot/mem.c: total usable main mem pages = %p\n",
	       (void *) total_pages);

	return total_pages;

end:
	return 0;
}
