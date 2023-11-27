#pragma once

void init_allocator();

#define USER_SEG_BASE reinterpret_cast<void*>(0xC0800000)
