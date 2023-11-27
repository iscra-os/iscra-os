#pragma once
 
enum segment_index {
    segm_null = 0,
    segm_kernel_code = 1,
    segm_kernel_data = 2,
    segm_user_code = 3,
    segm_user_data = 4,
    segm_task = 5
};

enum segment_value {
    kernel_cs = (segm_kernel_code * sizeof(uint64_t)) | 0,
    kernel_ds = (segm_kernel_data * sizeof(uint64_t)) | 0,
    user_cs = (segm_user_code * sizeof(uint64_t)) | 3,
    user_ds = (segm_user_data * sizeof(uint64_t)) | 3,
    tss_value = (segm_task * sizeof(uint64_t)) | 0
};

void init_gdt();