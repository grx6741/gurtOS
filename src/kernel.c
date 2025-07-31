#include "gdt.h"
#include "tty.h"
#include "serial_port.h"
#include "ps2.h"

#if defined(__linux__)
#error "gurt want cross compiler"
#endif

#if !defined(__i386__)
#error "gurt want ix86-elf compiler"
#endif

void kernel_main(void) 
{
    gdt_init();

    tty_init();

	init_serial_port(COM1_PORT);

	tty_writestring("balls\n");

	// com1_writestring("fortnite");

	ps2_init();
}
