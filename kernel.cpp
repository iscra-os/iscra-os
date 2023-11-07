#include "allocator.h"
#include "terminal.h"
#include "gdt.h"
#include "interrupts.h"
#include "vfs.h"
 
#if !defined(__i386__)
#error "This kernel needs to be compiled with a ix86-elf compiler"
#endif


extern "C" void kernel_main(void) {
	init_allocator();

	init_gdt();

	init_interrupts();

    terminal_setcolor(vga_entry_color(VGA_COLOR_MAGENTA, VGA_COLOR_WHITE));
	init_terminal();

	init_vfs();

	printk("Hello, kernel World! %x\n", 1024);

	int* ptr = 0; 
	*ptr = 1;

	// while(1) {
	// 	uint8_t status = 0;
	// 	while (!(status & 1)) {
	// 		status = inb(0x64);
	// 	}
	// 	char key = inb(0x60);
	// 	printk("%x\n", key);
	// }
}