#pragma once
#include <cstdint>

void init_allocator(uint32_t& framebuffer_addr);

#define USER_SEG_BASE reinterpret_cast<void*>(0xC0800000)
