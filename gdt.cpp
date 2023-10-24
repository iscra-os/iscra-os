// Used for creating GDT segment descriptors in 64-bit integer form.
#include <cstdint>
 
// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)
 
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed
 
#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR
 
#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_DATA_RDWR
 
uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
    uint64_t descriptor;
    
    // Create the high 32 bit segment
    descriptor  =  limit       & 0x000F0000;         // set limit bits 19:16
    descriptor |= (flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
    descriptor |=  base        & 0xFF000000;         // set base bits 31:24
 
    // Shift by 32 to allow for low part of segment
    descriptor <<= 32;
 
    // Create the low 32 bit segment
    descriptor |= base  << 16;                       // set base bits 15:0
    descriptor |= limit  & 0x0000FFFF;               // set limit bits 15:0
    
    return descriptor;
}

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

uint64_t gdt_table[5];

// 0-15     size
// 16-31    offset [low 16]   
// 32-47    offset [high 16]

extern "C" void init_gdt() {
    gdt_table[segm_null] = create_descriptor(0, 0, 0);
    gdt_table[segm_kernel_code] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
    gdt_table[segm_kernel_data] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
    gdt_table[segm_user_code] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL3));
    gdt_table[segm_user_data] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL3));

    
    struct [[gnu::packed]] {
        uint16_t size;
        uint32_t offset;
    } gdtr {
        sizeof(gdt_table) - 1,
        reinterpret_cast<uint32_t>(&gdt_table)
    };

    asm volatile("lgdt %0" :: "m"(gdtr));
    asm volatile(R"(
        ljmp %0, $reload_cs
    reload_cs:
    )" :: "i"(kernel_cs)); 
    asm volatile("movl %0, %%ds" :: "r"(kernel_ds));
    asm volatile("movl %0, %%es" :: "r"(kernel_ds));
    asm volatile("movl %0, %%fs" :: "r"(kernel_ds));
    asm volatile("movl %0, %%gs" :: "r"(kernel_ds));
    asm volatile("movl %0, %%ss" :: "r"(kernel_ds));
}