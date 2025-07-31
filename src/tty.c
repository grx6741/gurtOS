#include "tty.h"
#include "stdlib.h"

static size_t	 tty_row;
static size_t	 tty_column;
static uint8_t	 tty_color;
static uint16_t* tty_buffer = (uint16_t*)VGA_MEMORY;

uint8_t vga_entry_color(vga_color fg, vga_color bg)
{
	return (bg << 4) | fg;
}

uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return ((uint16_t) color << 8) | (uint16_t) uc;
}


void tty_init(void)
{
	tty_row = 0;
	tty_column = 0;
	tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			tty_buffer[index] = vga_entry(' ', tty_color);
		}
	}
}

void tty_setcolor(uint8_t color)
{
	tty_color = color;
}

void tty_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	tty_buffer[index] = vga_entry(c, color);
}

void tty_putchar(char c)
{
	if (c == '\n') {
		tty_column = 0;
		if (++tty_row == VGA_HEIGHT)
			tty_row = 0;

		return;
	}

	tty_putentryat(c, tty_color, tty_column, tty_row);
	if (++tty_column == VGA_WIDTH) {
		tty_column = 0;
		if (++tty_row == VGA_HEIGHT)
			tty_row = 0;
	}
}

void tty_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		tty_putchar(data[i]);
}

void tty_writestring(const char* data)
{
	tty_write(data, strlen(data));
}

