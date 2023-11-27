#include "processes.h"
#include <list>
#include <cstdint>
#include "terminal.h"
#include <vector>
#include "gdt.h"
#include "allocator.h"
#include <string.h>

struct register_frame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};

struct process {
    register_frame rframe;
    uint32_t eip;

    uint32_t cs;
    uint32_t ds;
    uint32_t eflags; 

    std::vector<uint8_t> stack;


};

process* current_process;
std::list<process*> proccesses;

process* create_process(uint32_t start, size_t allocate_stack, bool is_ring0) {
    auto new_process = new process();

    if (allocate_stack) {
        new_process->stack.resize(allocate_stack);
        new_process->rframe.esp = reinterpret_cast<uint32_t>(new_process->stack.data() + allocate_stack);
    }

    new_process->eip = start;

    uint32_t cs = user_cs;
    uint32_t ds = user_ds;
    if (is_ring0) {
        cs = kernel_cs;
        ds = kernel_ds;
    }
    new_process->cs = cs;
    new_process->ds = ds;
    

    proccesses.push_back(new_process);
    return new_process;
}

class StackOperator {
    uint8_t* _ptr;
public:
    StackOperator(uint8_t* ptr) : _ptr(ptr) {}

    template <typename T>
    uint8_t* push(const T& value) {
        auto ptr = reinterpret_cast<T*>(_ptr);
        --ptr;
        *ptr = value;
        _ptr = reinterpret_cast<uint8_t*>(ptr);
        return _ptr;
    }
};


struct interrupt_frame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags; 
};

struct interrupt_frame_ext {
    interrupt_frame base;
    uint32_t esp;
    uint32_t ss;
};
void exit_to_current_process() {
    bool is_ring0 = (current_process->cs & 0b11) == 0;

    StackOperator so(reinterpret_cast<uint8_t*>(current_process->rframe.esp));

    interrupt_frame iframe_base {
        current_process->eip,
        current_process->cs,
        current_process->eflags
    };    
    if (is_ring0) {
        so.push(iframe_base);
    } else {
        interrupt_frame_ext iframe {
            iframe_base,
            current_process->rframe.esp,
            current_process->ds
        };
        so.push(iframe);
    }

    auto esp = so.push(current_process->rframe);

    asm volatile(R"(
        movl %0, %%esp
        popa
        iret 
    )" :: "m"(esp));
}

void yield_internal(register_frame rframe, uint32_t eip) {
    current_process->rframe = rframe;
    current_process->rframe.esp += sizeof(uint32_t);
    current_process->eip = eip;

    proccesses.splice(proccesses.end(), proccesses, proccesses.begin());

    current_process = proccesses.front();

    exit_to_current_process();
}

[[gnu::naked]]
void yield() {
    asm volatile(R"(
        pusha
        call %P0
    )" :: "i"(yield_internal));
}
//          push (8*4)(%%esp)  ; push(*(esp + 8*4))


void func_p2() {
    printk("I was called\n");
    yield();
    printk("I was called 2\n");
    while (true)
        yield();
}

void init_processes() {
    new (&proccesses) std::list<process*>;
    
    current_process = create_process(0, 0, true);

    
    memcpy(
        USER_SEG_BASE, 
        reinterpret_cast<void*>(func_p2),
        256
        );
    
    auto p2 = create_process(reinterpret_cast<uint32_t>(USER_SEG_BASE), 0, false);
    p2->rframe.esp = reinterpret_cast<uint32_t>(USER_SEG_BASE) + 256 + 1024;

    // <- esp
    // reg7
    // reg6
    // reg5
    // reg4
    // reg3
    // reg2
    // reg1
    // reg0 <-
    // return address <-
    // 4
    // 7


    printk("I was called A\n");
    yield();
    printk("I was called B\n");
    yield();

    printk("All done\n");
}
