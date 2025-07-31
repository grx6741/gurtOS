#pragma once

#include <stdint.h>

#define GATE_TYPE_TASK				0b0101 // 0x5
#define GATE_TYPE_16_BIT_INTERRUPT  0b0110 // 0x6
#define GATE_TYPE_16_BIT_TRAP		0b0111 // 0x7
#define GATE_TYPE_32_BIT_INTERRUPT  0b1110 // 0xE
#define GATE_TYPE_32_BIT_TRAP		0b1111 // 0xF

typedef union {
	uint8_t whole;
	struct {
		uint8_t gate_type	: 4; // Use GATE_TYPE_*
		uint8_t zero		: 1;
		uint8_t dpl			: 2; // i think its safe to set this to 0
		uint8_t present		: 1; // must be set to 1
	};
} __attribute__((packed)) idt_descriptor_flag_t;

typedef struct {
	uint16_t				isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    			kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     			reserved;     // Set to zero
	idt_descriptor_flag_t	attributes;   // Type and attributes; see the IDT page
	uint16_t				isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_descriptor_t;

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) idtr_t;

typedef __attribute__((interrupt)) void (*isr_handler_t)(void*);

void init_idt();
idt_descriptor_t set_idt(isr_handler_t isr, idt_descriptor_flag_t flags);
