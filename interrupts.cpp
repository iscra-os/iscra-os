#include <cstdint>
#include <utility>

extern "C" int printk(const char* fmt, ...);


void exception_handler_body(int n, ...) {
    printk("Exception occured (vec = %d)", n);
    while(1);
} 

void intterupt_handler_body(int n, ...) {
    printk("Interrupt occured (vec = %d)", n);
} 

template <int I>
[[gnu::naked]]
void intterupt_handler() {
    asm volatile(R"(
        push %0
        call %P1
        addl $4, %%esp
        iret
    )" :: "i"(I), "i"(I >= 32 ? intterupt_handler_body : exception_handler_body));
}

uint64_t idt_table[256];


enum segment_index {
    segm_null = 0,
    segm_kernel_code = 1,
    segm_kernel_data = 2,
    segm_user_code = 3,
    segm_user_data = 4
};

enum segment_value {
    kernel_cs = (segm_kernel_code * sizeof(uint64_t)) | 0,
    kernel_ds = (segm_kernel_data * sizeof(uint64_t)) | 0,
    user_cs = (segm_user_code * sizeof(uint64_t)) | 3,
    user_ds = (segm_user_data * sizeof(uint64_t)) | 3
};

enum gate_type {
    gate_int32 = 0xE
};

uint64_t create_descriptor(uint32_t offset, uint16_t selector, uint8_t dpl, gate_type gt) {
    uint64_t descriptor;
    // 31 -- 16 15 -- 0 >> 16
    // aaaaaaaa bbbbbbb >> 16
    // ------------------
    // 00000000 aaaaaaaa
    descriptor = (uint16_t)offset; 
    descriptor |= selector << 16; 
    descriptor |= (uint64_t)gt << 40; 
    descriptor |= (uint64_t)dpl << 45; 
    descriptor |= (uint64_t)1 << 47;
    descriptor |= (uint64_t)(offset >> 16) << 48;  

    return descriptor;
}


template <int ... I>
void init_interrupts_body(std::integer_sequence<int, I...> const &) {
    ((idt_table[I] = create_descriptor(
        reinterpret_cast<uint32_t>(intterupt_handler<I>),
        kernel_cs,
        0,
        gate_int32
    )), ...);
}


extern "C" void init_interrupts() {
    init_interrupts_body(std::make_integer_sequence<int, 256>{});

    struct [[gnu::packed]] {
        uint16_t size;
        uint32_t offset;
    } idtr {
        sizeof(idt_table) - 1,
        reinterpret_cast<uint32_t>(&idt_table)
    };

    asm volatile("lidt %0" :: "m"(idtr));

    int* ptr = 0;
    *ptr = 0;
}