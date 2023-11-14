#include "processes.h"
#include <list>
#include <cstdint>
#include "terminal.h"

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
};

process* current_process;
std::list<process*> proccesses;


void exit_to_current_process() {
    auto eip_stack_ptr = reinterpret_cast<uint32_t*>(current_process->rframe.esp);

    // <- esp
    // eip
    
    --eip_stack_ptr;
    *eip_stack_ptr = current_process->eip;

    auto rframe_stack_ptr = reinterpret_cast<register_frame*>(eip_stack_ptr);
    --rframe_stack_ptr; // -8*4 
    *rframe_stack_ptr = current_process->rframe;

    asm volatile(R"(
        movl %0, %%esp
        popa
        ret 
    )" :: "m"(rframe_stack_ptr));
}

void yield_internal(register_frame rframe, uint32_t eip) {
    printk("%08X\n", rframe.eax);

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

char stack[256];

void func_p2() {
    printk("I was called\n");
    yield();
    printk("I was called 2\n");
    while (true)
        yield();
}

void init_processes() {
    new (&proccesses) std::list<process*>;
    
    current_process = new process();
    proccesses.push_back(current_process);

    auto p2 = new process();
    p2->eip = reinterpret_cast<uint32_t>(func_p2);
    p2->rframe.esp = reinterpret_cast<uint32_t>(stack + sizeof(stack));
    proccesses.push_back(p2);

    
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

    printk("x");
}
