#include <cstdint>
#include "terminal.h"

extern uint32_t boot_page_directory[1024];
[[gnu::aligned(4096)]]
uint32_t memory_page_table[1024];
[[gnu::aligned(4096)]]
uint32_t user_page_table[1024];
[[gnu::aligned(4096)]]
uint32_t framebuffer_page_table[1024];
// 0xC0000000 to 0xC0400000 - kernel (768)
// 0xC0400000 to 0xC0800000 - heap (769)
// 0xC0800000 to 0xC0C00000 - user (770)
// 0xC0C00000 to 0xC1000000 - framebuffer (771)

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

void init_allocator(uint32_t& framebuffer_addr) {
    
	for (size_t i = 0; i < 1024; ++i) {
		memory_page_table[i] = (0x400000 + i * 4096) | 0b11;
	}
	for (size_t i = 0; i < 1024; ++i) {
		user_page_table[i] = (0x800000 + i * 4096) | 0b111;
	}
	for (size_t i = 0; i < 1024; ++i) {
		framebuffer_page_table[i] = (framebuffer_addr + i * 4096) | 0b11;
	}
	boot_page_directory[769] = ((uint32_t)memory_page_table - 0xC0000000) | 0b11;
	boot_page_directory[770] = ((uint32_t)user_page_table - 0xC0000000) | 0b111;
	boot_page_directory[771] = ((uint32_t)framebuffer_page_table - 0xC0000000) | 0b11;

	program_break = (uint8_t*)0xC0400000;
	program_break_limit = (uint8_t*)0xC0800000;
	
	framebuffer_addr = 0xC0C00000;
}