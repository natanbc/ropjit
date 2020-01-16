#include "ropjit.h"
#include <stdio.h>
#include <sys/syscall.h> //for SYS_x defs

// syscall args (syscall number goes into rax)
// x86-64        rdi   rsi   rdx   r10   r8    r9    -
// SYS_write
// ssize_t write(int fd, const void *buf, size_t count);

int main(void) {
    uintptr_t data[100];
    
    data[0] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[1] = SYS_write;
    
    data[2] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDI);
    data[3] = 1;

    data[4] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RSI);
    data[5] = (uintptr_t)"Hello World\n";
    
    data[6] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDX);
    data[7] = 12; // strlen("Hello World\n");
    
    data[8] = (uintptr_t)ropjit_syscall;

    //return from jit code. ropjit_run's second argument is a pointer to store
    //the stack pointer from the original stack, and moving it back into RSP will
    //exit from jit code.
    //
    //another option that would allow multiple exit locations would be
    //  uintptr_t originalStack;
    //  data[9] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    //  data[10] = (uintptr_t)&originalStack;
    //  data[11] = (uintptr_t)ropjit_gadget_load(ROPJIT_REG_RSP, ROPJIT_REG_RAX);
    //  ropjit_run(data, &originalStack);
    data[9] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[10] = 0; //dummy, will be replaced in ropjit_run
    data[11] = (uintptr_t)ropjit_gadget_move(ROPJIT_REG_RSP, ROPJIT_REG_RAX);

    printf("before ropjit_run\n");
    ropjit_run(data, &data[10]);
    printf("after  ropjit_run\n");

    return 0;
}
