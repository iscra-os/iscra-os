#include "terminal.h"
#include "string.h"
#include <cstdarg>
#include <cstdio>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) {
	if (c == '\n')
		terminal_column = VGA_WIDTH - 1;
	else
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

int vprintk(const char* fmt, va_list list) {
	char buffer[512];

	int result = vsnprintf(buffer, sizeof(buffer), fmt, list);

	if (result < 0) {
		terminal_writestring("Failed to format\n");
		return result;
	}
	else if (result == sizeof(buffer)) {
		terminal_writestring("String is too long\n");
		buffer[sizeof(buffer) - 1] = '\0';
	}

	terminal_writestring(buffer);

	return result;
}

int printk(const char* fmt, ...) {
	va_list list;

	va_start(list, fmt);
	int result = vprintk(fmt, list);
	va_end(list);

	return result;
}

[[noreturn]]
void panic(const char* fmt, ...) {
	va_list list;

	va_start(list, fmt);
	int result = vprintk(fmt, list);
	va_end(list);

	while (true)
		asm volatile("hlt"); // halt
}




void init_terminal(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_buffer = (uint16_t*)0xC03FF000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
