/*
 * gdt.c
 * -----
 * intialize the global descriptor table (GDT).
 */
#include <boot/gdt.h>

extern void load_gdt(struct gdt_desc_struct *gdt_desc);

struct gdt_segment_desc_struct gdt_entries[MAX_GDT_DESC];
struct gdt_desc_struct gdt_desc;

/*
 * notes:
 *  - In 64-bit mode, the Base and Limit values are ignored, each
 * descriptor covers the entire linear address space regardless of what
 * they are set to.
 *  - gdt descriptor table limit: we subtract 1 because the maximum
 * value of limit (i.e. size) is 65535, while the GDT can be up to 65536 bytes in
 * length (8192 entries). Further, no GDT can have a limit of 0 bytes.
 */
void init_gdt(void)
{
	/* initialize the gdt descriptor struct variable */

	/* The size of the table in bytes subtracted by 1. */
	gdt_desc.table_limit = (sizeof(struct gdt_segment_desc_struct)
				* MAX_GDT_DESC) - 1;
	gdt_desc.base = gdt_entries;


	/* create the descriptors */
	create_descriptor(0, 0, 0, 0, 0);	/* null descriptor */
	create_descriptor(1, 0, 0, 0x9a, 0xa);	/* kernel mode code segment */
						/* descriptor. */
	create_descriptor(2, 0, 0, 0x92, 0xc);	/* kernel mode data segment */
						/* descriptor. */

	load_gdt(&gdt_desc);
}

static void create_descriptor(int index, uint32_t base, uint32_t limit,
			      uint8_t access_byte, uint8_t flags)
{
	gdt_entries[index].base_addr_low = base;
	gdt_entries[index].base_addr_mid = base;
	gdt_entries[index].base_addr_high = base;
	gdt_entries[index].segment_limit_low = limit;
	gdt_entries[index].access_byte = access_byte;
	gdt_entries[index].flags_and_segment_limit_high = (flags << 4);
						/* segment limit high bits */
						/* are 0 (ignored). */
}
