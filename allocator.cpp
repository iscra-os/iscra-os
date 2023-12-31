#include <cstdint>
#include "terminal.h"

extern uint32_t memory_page_table[1024];
extern uint32_t boot_page_directory[1024];
// 0xC0000000 to 0xC0400000 - kernel (768)
// 0xC0400000 to 0xC0800000 - heap (769)

uint8_t* program_break;
uint8_t* program_break_limit;

extern "C" void *sbrk(intptr_t increment) {
	void* ptr = program_break;

	program_break += increment;

	if (program_break >= program_break_limit) {
		terminal_writestring("Out of memory\n");
		while(1);
	}

	return ptr;
}

void init_allocator() {
    
	for (size_t i = 0; i < 1024; ++i) {
		memory_page_table[i] = (0x400000 + i * 4096) | 0b11;
	}
	boot_page_directory[769] = ((uint32_t)memory_page_table - 0xC0000000) | 0b11;

	program_break = (uint8_t*)0xC0400000;
	program_break_limit = (uint8_t*)0xC0800000;
    
}