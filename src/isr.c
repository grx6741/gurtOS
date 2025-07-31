#include "isr.h"

void isr_exception_handler(void* frame) {
	(void)frame;

	__asm__ volatile ("cli; hlt"); // Completely hangs the computer
	while (1); // To stop my dumb LSP from shouting
}
