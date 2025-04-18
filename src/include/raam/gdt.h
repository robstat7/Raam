#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define MAX_GDT_DESC			3 /* null, kernel code and data */
					  /* segment descriptors. */

struct gdt_segment_desc_struct {
	uint16_t segment_limit_low;
	uint16_t base_addr_low;
	uint8_t base_addr_mid;
	uint8_t access_byte;
	uint8_t flags_and_segment_limit_high; /* includes 4-bit flags and */
					      /* 4-bit segment limit high */
					      /* bits. */
	uint8_t base_addr_high;
}__attribute__((packed));

struct gdt_desc_struct {                                                                
	uint16_t table_limit;                                                         
        struct gdt_segment_desc_struct *base;
} __attribute__((packed));

static void create_descriptor(int index, uint32_t base, uint32_t limit,         
                              uint8_t access_byte, uint8_t flags);
void init_gdt(void);

#endif	/* GDT_H */
