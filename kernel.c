#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <stdlib.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	// terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xC03FF000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) 
{
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
 
void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

int close(int fd) {
	terminal_writestring("close()\n");
	while (1);
}

off_t lseek(int fd, off_t offset, int whence) {
	terminal_writestring("lseek()\n");
	while (1);
}
ssize_t read(int fd, void *buf, size_t count) {
	terminal_writestring("read()\n");
	while (1);
}

ssize_t write(int fd, const void *buf, size_t count) {
	terminal_writestring("write()\n");
	while (1);
}


int printk(const char* fmt, ...) {
	char buffer[512];
	va_list list;

	va_start(list, fmt);
	int result = vsnprintf(buffer, sizeof(buffer), fmt, list);
	va_end(list);

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

extern uint32_t memory_page_table[1024];
extern uint32_t boot_page_directory[1024];
// 0xC0000000 to 0xC0400000 - kernel (768)
// 0xC0400000 to 0xC0800000 - heap (769)

uint8_t* program_break;
uint8_t* program_break_limit;

void *sbrk(intptr_t increment) {
	void* ptr = program_break;

	program_break += increment;

	if (program_break >= program_break_limit) {
		terminal_writestring("Out of memory\n");
		while(1);
	}

	return ptr;
}

uint8_t inb(uint16_t port) {
	uint8_t data;
	asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port) );
	return data;
}

void outb(uint16_t port, uint8_t data) {
	asm volatile("outb %0, %1" :: "a"(data), "Nd"(port) );
}


void kernel_main(void) 
{
	/* Initialize terminal interface */
    terminal_setcolor(vga_entry_color(VGA_COLOR_MAGENTA, VGA_COLOR_WHITE));
	terminal_initialize();
	/* Newline support is left as an exercise. */
	printk("Hello, kernel World! %x\n", 1024);

	for (size_t i = 0; i < 1024; ++i) {
		memory_page_table[i] = (0x400000 + i * 4096) | 0b11;
	}
	boot_page_directory[769] = ((uint32_t)memory_page_table - 0xC0000000) | 0b11;

	program_break = (uint8_t*)0xC0400000;
	program_break_limit = (uint8_t*)0xC0800000;

	while(1) {
		uint8_t status = 0;
		while (!(status & 1)) {
			status = inb(0x64);
		}
		char key = inb(0x60);
		printk("%x\n", key);
	}
}