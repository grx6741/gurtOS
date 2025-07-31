#include "idt.h"

#include "isr.h"

#define GDT_OFFSET_KERNEL_CODE 0x8
#define IDT_MAX_DESCRIPTORS 256

__attribute__((aligned(0x10))) // 16 bit alligned
static idt_descriptor_t idt[IDT_MAX_DESCRIPTORS];

	static idtr_t idtr;

idt_descriptor_t set_idt(isr_handler_t isr, idt_descriptor_flag_t flags)
{
	return (idt_descriptor_t) {
		.isr_low        = (uint32_t)isr & 0xFFFF,
		.kernel_cs      = GDT_OFFSET_KERNEL_CODE,
		.attributes     = flags,
		.isr_high       = ((uint32_t)isr >> 16) & 0xFFFF,
		.reserved       = 0,
	};
}

void init_idt()
{
	idtr.base = (uintptr_t)&idt[0];
	idtr.limit = (uint16_t)sizeof(idt_descriptor_t) * IDT_MAX_DESCRIPTORS - 1;

	for (uint16_t i = 0; i < IDT_MAX_DESCRIPTORS; i++)
	{
		idt[i] = set_idt(&isr_exception_handler, (idt_descriptor_flag_t) {
					.gate_type = GATE_TYPE_32_BIT_INTERRUPT,
					.present = 1
				});
	}

	__asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
	__asm__ volatile ("sti"); // set the interrupt flag
}
