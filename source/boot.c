/*
 * Raam Raam sa _/\_ _/\_ _/\_
 *
 * Boot loader written using gnu-efi
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "fb.h"
#include "mm.h"
#include "paging.h"
#include "printk.h"

uint8_t *sys_var_ptr;

int8_t validate_xsdp_checksum(void *table);
void find_reserved_mem(UINTN msize, uint8_t mmap[], UINTN dsize);
int get_mem_map(UINTN *msize, uint8_t *mmap, UINTN *mkey, UINTN *dsize);
void get_ram_attrs(UINTN msize, uint8_t mmap[], UINTN dsize, uint64_t ** physical_start_addr_ptr, uint64_t ** physical_end_addr_ptr, uint64_t *ram_size);
int create_bitmap(UINTN msize, uint8_t mmap[], UINTN dsize, uint8_t **bitmap_ptr_ptr, uint64_t *bitmap_size_ptr);
int allocate_sys_variables_mem(uint8_t **sys_var_ptr_ptr);
void setup_and_enable_paging(void);
volatile uint64_t *find_first_4096_byte_aligned_address(char *sys_var_ptr);

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS Status;
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo, numModes;
	int i;
	uint32_t mode;
	struct frame_buffer_descriptor frame_buffer;
	struct memory_map memory_map_uefi;
	int num_config_tables;
	EFI_CONFIGURATION_TABLE *config_tables;
	EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;	/* EFI GUID for a pointer to the ACPI 2.0 or later specification XSDP structure */
	void *table;
	void *xsdp;
	uint8_t revision;

	xsdp = NULL;
	table = NULL;
	

	InitializeLib(ImageHandle, SystemTable);

	/* detecting GOP */
	Status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(Status))
		Print(L"error: unable to locate GOP!\n");

	/* get the current mode */
  	Status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
  	/* this is needed to get the current video mode */
  	if (Status == EFI_NOT_STARTED)
  		Status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
  	if(EFI_ERROR(Status)) {
  		Print(L"error: unable to get native video mode!\n");
  	} else {
  		numModes = gop->Mode->MaxMode;
  	}

	/* query available video modes and get the mode number for 1280 x 1024 resolution */
	mode = -1;
	for (i = 0; i < numModes; i++) {
		Status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
		if (info->HorizontalResolution == 1280 && info->VerticalResolution == 1024 && info->PixelFormat == 1) {
			mode = i;
			break;
		}
  	}

	if (mode == -1) {
		Print(L"error: unable to get video mode for 1280 x 1024 resolution!\n");
	} else {
		/* set video mode and get the framebuffer */
		Status = uefi_call_wrapper(gop->SetMode, 2, gop, mode);
		if(EFI_ERROR(Status)) {
			Print(L"error: unable to set video mode %03d\n", mode);
		} else {
			/* store framebuffer information */
			frame_buffer.frame_buffer_base = (long unsigned int *) gop->Mode->FrameBufferBase;
			frame_buffer.frame_buffer_size = gop->Mode->FrameBufferSize;
			frame_buffer.horizontal_resolution = gop->Mode->Info->HorizontalResolution;
			frame_buffer.vertical_resolution = gop->Mode->Info->VerticalResolution;
			frame_buffer.pixels_per_scan_line = gop->Mode->Info->PixelsPerScanLine;
		}
	}

	// Print(L"frame_buffer_base = %p\n", (void *) frame_buffer.frame_buffer_base);


	/* locating and storing the pointer to the XSDP structure */	
	num_config_tables = SystemTable->NumberOfTableEntries;

	config_tables = SystemTable->ConfigurationTable;	

	for(i = 0; i < num_config_tables; i++) {
		if (CompareGuid(&config_tables[i].VendorGuid, &Acpi20TableGuid) == 0) {
			table = config_tables[i].VendorTable;
			/* validate the XSDP */
			/* check if ACPI version is >= 2.0 */
			revision = *((uint8_t *) table + 15);
			if(revision == 0x2) {
				/* validate checksum */
				if(validate_xsdp_checksum(table) == 0) {
					/* store xsdp struct pointer */
					xsdp = table;
					break;
				}
			}
		}
	}

	if(xsdp == NULL) {
		Print(L"error: could not find XSDP structure pointer!\n");
		goto end;
	}


 	// /* get memory map */
	// memory_map_uefi.msize = sizeof(memory_map_uefi.mmap);
	// memory_map_uefi.mkey = 0;
	// memory_map_uefi.dsize = 0;

	// if(get_mem_map(&memory_map_uefi.msize, memory_map_uefi.mmap, &memory_map_uefi.mkey, &memory_map_uefi.dsize) == 1) {
 	// 	goto end;
 	// }

	// /* find the reserved memory */
	// find_reserved_mem(msize, mmap, dsize);
	
	// uint8_t *bitmap_ptr;
	// uint64_t bitmap_size;

	// if(create_bitmap(memory_map_uefi.msize, memory_map_uefi.mmap, memory_map_uefi.dsize, &bitmap_ptr, &bitmap_size) == 1) {
	// 	goto end;
	// }

	// Print(L"@bitmap = %p\n", (void *) bitmap_ptr);
	// Print(L"bitmap_size = %llu\n", bitmap_size);

	
	/* allocate and clear 2 MiB of memory for system variables */

	if(allocate_sys_variables_mem(&sys_var_ptr) == 1) {
		goto end;
	}


	// Print(L"@sys_var_ptr= %p\n", (void *) sys_var_ptr);

	

	/* get memory map */	
	memory_map_uefi.msize = sizeof(memory_map_uefi.mmap);
	memory_map_uefi.mkey = 0;
	memory_map_uefi.dsize = 0;

	if(get_mem_map(&memory_map_uefi.msize, memory_map_uefi.mmap, &memory_map_uefi.mkey, &memory_map_uefi.dsize) == 1) {
		goto end;
	}

	/* try to exit boot services 3 times */
  	for (i = 0; i < 3; i++) {
		/* exit boot services */
		Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, memory_map_uefi.mkey);
		if (Status == EFI_SUCCESS) {
			break;
		}
	}
	if(Status == EFI_INVALID_PARAMETER) {
		Print(L"error: exit boot services: map key is incorrect!\n");
		goto end;
	}


	/* initialize terminal output */
	// tty_out_init(frame_buffer);
	// printk("hello");

	/* fill terminal background color with white */
	// fill_tty_bgcolor();
	
	/* initialize gdt */
	init_gdt();

	/* initialize idt */
	init_idt();
	
	// setup and enable paging

	setup_and_enable_paging();

	// Print(L"@done enabling paging\n");


	*(volatile uint32_t *) 0x4000000000 = 0xff0000;

	/* jump to core */
	// main(xsdp, sys_var_ptr); // use this call atm
	// main(xsdp, &memory_map_uefi);


	/* should not reach here */
end:
	/* hang here */
	for(;;);

	return 1;
}

/*
 * returns 0 if the checksum is valid.
 */
int8_t validate_xsdp_checksum(void *table)
{
	uint8_t rsdp_start_offset;
	uint8_t xsdp_start_offset;
	uint8_t rsdp_end_offset;
	uint8_t xsdp_end_offset;
	int sum;
	uint8_t i;

	rsdp_start_offset = 0;
	rsdp_end_offset= 19;
	xsdp_start_offset = 20;
	xsdp_end_offset = 35;
	sum = 0;

	for(i = rsdp_start_offset; i <= rsdp_end_offset; i++)
		sum += *((int8_t *) table + i);

	if((int8_t) sum == 0) {
		for(i = xsdp_start_offset; i <= xsdp_end_offset; i++)
			sum += *((int8_t *) table + i);
	}

	return (int8_t) sum;
}

int get_mem_map(UINTN *msize, uint8_t *mmap, UINTN *mkey, UINTN *dsize)
{
	EFI_STATUS Status = uefi_call_wrapper(BS->GetMemoryMap, 5, msize, mmap, mkey, dsize, NULL);

	if(EFI_ERROR(Status)) {
		Print(L"error: could not get memory map!\n");
		return 1;
	}

	return 0;
}

/* allocate and clear 2 MiB of memory for system variables */
int allocate_sys_variables_mem(uint8_t **sys_var_ptr_ptr)
{
	// EFI_STATUS Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiRuntimeServicesData, 2097152, (void **) sys_var_ptr_ptr);
	EFI_STATUS Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, 2097152, (void **) sys_var_ptr_ptr);
    	if (EFI_ERROR(Status)) {
             Print(L"error: allocate_pool: out of pool  %x!\n", Status);
             *sys_var_ptr_ptr = NULL;
	     return 1;
    	}


	/* fill the bitmap buffer with 0 */	
	uefi_call_wrapper(BS->SetMem, 3, (void *) *sys_var_ptr_ptr, 2097152, 0);

	return 0;
}

int create_bitmap(UINTN msize, uint8_t mmap[], UINTN dsize, uint8_t **bitmap_ptr_ptr, uint64_t * bitmap_size_ptr)
{
	uint64_t *physical_start_addr = (uint64_t *) UINT64_MAX, *physical_end_addr = (uint64_t *) 0, ram_size;
	
	get_ram_attrs(msize, mmap, dsize, &physical_start_addr, &physical_end_addr, &ram_size);

	const uint64_t bitmap_size = ram_size/PAGE_SIZE;

	*bitmap_size_ptr = bitmap_size;
	
	EFI_STATUS Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, bitmap_size, (void **) bitmap_ptr_ptr);
    	if (EFI_ERROR(Status)) {
        	Print(L"error: allocate_pool: out of pool  %x!\n", Status);
        	*bitmap_ptr_ptr = NULL;
		return 1;
    	}

	/* fill the bitmap buffer with 0 */	
	uefi_call_wrapper(BS->SetMem, 3, (void *) *bitmap_ptr_ptr, bitmap_size, 0);

	return 0;
}


void get_ram_attrs(UINTN msize, uint8_t mmap[], UINTN dsize, uint64_t ** physical_start_addr_ptr, uint64_t ** physical_end_addr_ptr, uint64_t *ram_size)
{
	int num_desc = msize/dsize;

	for(int i = 0; i < num_desc; i++) {
		EFI_MEMORY_DESCRIPTOR desc = *(EFI_MEMORY_DESCRIPTOR *) mmap;

		mmap += dsize;

		uint64_t *PhysicalEnd = (uint64_t *) (desc.PhysicalStart + (desc.NumberOfPages * 4096)) - 1;

		*physical_start_addr_ptr = ((uint64_t) desc.PhysicalStart < (uint64_t) *physical_start_addr_ptr) ? (uint64_t *) desc.PhysicalStart : *physical_start_addr_ptr;
		*physical_end_addr_ptr = ((uint64_t) PhysicalEnd > (uint64_t) *physical_end_addr_ptr) ? PhysicalEnd : *physical_end_addr_ptr;
	}
	*ram_size = (uint64_t) *physical_end_addr_ptr - (uint64_t) *physical_start_addr_ptr;
}

/*
 * find the reserved memory
 *
 * do not modify the memory map in this function
 */
void find_reserved_mem(UINTN msize, uint8_t mmap[], UINTN dsize)
{
	int num_desc;

	Print(L"descriptor size=%llu\n", dsize);
	Print(L"msize=%llu\n\n", msize);

	num_desc = msize/dsize;

	for(int i = 0; i < num_desc; i++) {
		EFI_MEMORY_DESCRIPTOR desc = *(EFI_MEMORY_DESCRIPTOR *) mmap;
		if(desc.Type == EfiLoaderCode || desc.Type == EfiLoaderData ||
		   desc.Type == EfiUnusableMemory ||
		   desc.Type == EfiACPIReclaimMemory ||
		   desc.Type == EfiACPIMemoryNVS ||
		   desc.Type == EfiMemoryMappedIO ||
		   desc.Type == EfiMemoryMappedIOPortSpace ||
		   desc.Type == EfiPalCode ||
		   desc.Type == EfiReservedMemoryType) {
			Print(L"physical_start=%llu\n", desc.PhysicalStart);
			// UINT64 physical_end = (desc.PhysicalStart + (desc.NumberOfPages * 4096)) - 1;
			Print(L"number of 4 KiB pages=%llu\n", desc.NumberOfPages);
		}
		mmap += dsize;
	}
}

void load_pml4_table(pml4_table_t *phy_addr)
{
	__asm__ __volatile__("mov cr3, %0"::"r"(phy_addr));
}

void enable_pae()
{

	// PAE and PGE
	__asm__ __volatile__("mov rax, cr4;"
			     "or rax, 0x20;"
			     "mov cr4, rax"::);
}

void page_disable()
{	__asm__("mov rax, cr0\n\t"
		"mov ebx, 0x7FFFFFFF\n\t"
		"and eax, ebx\n\t"
		"mov cr0, rax"
		:::"rax", "ebx");
}

void page_enable()
{	__asm__ __volatile__("mov rax, cr0;"
			     "mov rbx, 0x80000000;"
			     "or rax, rbx;"
			     "mov cr0, rax"::);
}

void setup_and_enable_paging(void)
{
	volatile char *addr_1 = (void *) find_first_4096_byte_aligned_address(sys_var_ptr);

	// Print(L"@setup_and_enable_paging: addr_1 = %p\n", (void *) addr_1);

	volatile pml4_table_t *pml4e = (pml4_table_t *)addr_1;

	volatile pae_page_directory_pointer_table_t *pdpt_base = (pae_page_directory_pointer_table_t*) ((char *) pml4e + 0x1000);

	volatile pae_page_directory_pointer_table_t *pdpte = (pae_page_directory_pointer_table_t*) ((char *) pdpt_base + (256 * 8));

	volatile pae_page_directory_table_t *pde = (pae_page_directory_table_t *) ((char *) pdpt_base + 0x1000);

	// pae_page_table_t *pt_base = (pae_page_table_t *) ((char *) pde + 0x1000);
	
	volatile pae_page_table_t *pte = (pae_page_table_t *) ((char *) pde + 0x1000);

	// pae_page_table_t *pte = (pae_page_table_t *) ((char *) pt_base + (16 * 8));


	unsigned long addr_2;
	uint32_t cpu_edx, cpu_eax;

	pml4e->p = 1;
	pml4e->rw = 1;
	pml4e->us = 1;
	addr_2 = (unsigned long) pdpt_base;
	pml4e->pdpt_phy_addr = (addr_2 >> 12) & 0x3FFFFFFFFF;
//	pml4e->pdpt_phy_addr = ((addr_2 & 0xFFF) << 4) | ((addr_2 >> 28) & 0xF);
	// pml4e->pdpt_phy_addr = addr_2;

	// Print(L"@pml4e->pdpt_phy_addr = %p\n", (void *) pml4e->pdpt_phy_addr);

	pdpte->p = 1;
	pdpte->rw = 1;
	pdpte->us = 1;
	unsigned long addr_3;
	addr_3 = (unsigned long) pde;
	// pdpte->pd_phy_addr = (addr_3 >> 12) & 0xFFFFF;
	pdpte->pd_phy_addr = (addr_3 >> 12) & 0x3FFFFFFFFF;
	// pdpte->pd_phy_addr = ((addr_3 & 0xFFF) << 4) | ((addr_3 >> 28) & 0xF);
	// pdpte->pd_phy_addr = addr_3;

	// Print(L"@pdpte->pd_phy_addr = %p\n", (void *) pdpte->pd_phy_addr);

	pde->p = 1;
	pde->rw = 1;
	pde->us = 1;
	// unsigned long addr_4 = (unsigned long) pt_base;
	unsigned long addr_4 = (unsigned long) pte;
	pde->pt_phy_addr = (addr_4 >> 12) & 0x3FFFFFFFFF;
	// pde->pt_phy_addr = ((addr_4 & 0xFFF) << 4) | ((addr_4 >> 28) & 0xF);
	// pde->pt_phy_addr = addr_4;

	// Print(L"@pde->pt_phy_addr = %p\n", (void *) pde->pt_phy_addr);

	pte->p = 1;
	pte->rw = 1;
	pte->us = 1;
	unsigned long addr_5 = (unsigned long) 0x4000000000;
	pte->page_4k_phy_addr = (addr_5 >> 12) & 0x3FFFFFFFFF;
	// pte->page_4k_phy_addr = ((addr_5 & 0xFFF) << 4) | ((addr_5 >> 28) & 0xF);
	// pte->page_4k_phy_addr = addr_5;

	// Print(L"@pte->page_4k_phy_addr = %p\n", (void *) pte->page_4k_phy_addr);




	__asm__ __volatile__("cli");

	// enable_paging(pml4e);
	//
	
	__asm__("mov cr3, %0"::"r" (pml4e));
	
	// __asm__("mov r8, %0\n\t"
	// 	"call enable_paging"
	// 	::"m" (pml4e):"r8");
	

//	Print(L"@debug\n");
	// page_disable();	
	// printk("@debug1\n");
	// enable_pae();
	// load_pml4_table(pml4e);

	//  ; Set LME (long mode enable)
	// setmsr(0xC0000080 , 0x100, 0);
	// read_msr_reg(0xC0000080 , &cpu_edx, &cpu_eax);
	// cpu_eax |= 0x100;
	// write_msr_reg(0xC0000080 , &cpu_edx, &cpu_eax);

	// printk("@debug2\n");
	// page_enable();

	__asm__ __volatile__("sti");
}

volatile uint64_t *find_first_4096_byte_aligned_address(char *sys_var_ptr)
{
	volatile char *addr = sys_var_ptr;

	while((uint64_t) addr % 4096 != 0) {
		addr++;
	}

	return (volatile uint64_t *) addr;
}
