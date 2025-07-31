#pragma once

#include <stdint.h>

typedef struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t  base_middle;
	uint8_t  access;
	uint8_t  granularity;
	uint8_t  base_high;
} __attribute__((packed)) gdt_descriptor_t;

typedef struct {
	gdt_descriptor_t null_desc;
	gdt_descriptor_t code_segment_desc;
	gdt_descriptor_t data_segment_desc;

	uint16_t limit;
	uintptr_t base;
} __attribute__((packed)) gdt_t;

// Defined in `set_gdt.s`
void gdt_set(uint16_t limit, uintptr_t base);

gdt_descriptor_t gdt_get_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

void gdt_init();
