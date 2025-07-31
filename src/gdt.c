#include "gdt.h"

gdt_descriptor_t gdt_get_descriptor(uint32_t base, 
				uint32_t limit, 
				uint8_t access, 
				uint8_t gran)
{
	return (gdt_descriptor_t) {
		.base_low    = (base & 0xFFFF),
		.base_middle = (base >> 16) & 0xFF,
		.base_high   = (base >> 24) & 0xFF,

		.limit_low   = (limit & 0xFFFF),
		.granularity = ( (limit >> 16) & 0x0F ) | ( gran & 0xF0 ),
		.access      = access,
	};
}

void gdt_init()
{
	static gdt_t gdt;

	gdt.null_desc		= gdt_get_descriptor(0, 0, 0, 0);
	gdt.code_segment_desc	= gdt_get_descriptor(0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt.data_segment_desc 	= gdt_get_descriptor(0, 0xFFFFFFFF, 0x92, 0xCF);

	gdt.limit = 3 * sizeof(gdt_descriptor_t) - 1;
	gdt.base = (uintptr_t) &gdt.null_desc;

	gdt_set(gdt.limit, gdt.base);
}
