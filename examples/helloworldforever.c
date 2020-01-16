#include "ropjit.h"
#include <stdio.h>
#include <sys/syscall.h> //for SYS_x defs

// syscall args (syscall number goes into rax)
// x86-64        rdi   rsi   rdx   r10   r8    r9    -
// SYS_write
// ssize_t write(int fd, const void *buf, size_t count);

int main(void) {
    uintptr_t data[100];
     
    data[0] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDI);
    data[1] = 1;

    data[2] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RSI);
    data[3] = (uintptr_t)"Hello World\n";
    
    data[4] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDX);
    data[5] = 12; // strlen("Hello World\n");
    
    data[6] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[7] = SYS_write;
    data[8] = (uintptr_t)ropjit_syscall;

    //jump back to the syscall. if you compare this file with the `helloworld`
    //example, you'll notice that rax is set right before the syscall, instead
    //of right at the start. this is done because the return value of a syscall
    //is put into rax on linux, so we need to overwrite it before each syscall.
    //doing it right before the syscall allows jumping back to data[6] instead
    //of data[0].
    //
    //as for how the jump works, it's done by overwriting the value of rsp with
    //the address of the wanted instruction, since nothing edits the stack. if
    //there were function calls or stack editing, they'd need to be undone. check
    //the `loop` example for how to do that.
    data[9] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_R8);
    data[10] = (uintptr_t)&data[6];
    data[11] = (uintptr_t)ropjit_gadget_move(ROPJIT_REG_RSP, ROPJIT_REG_R8);

    printf("before ropjit_run\n");
    uintptr_t ignored;
    ropjit_run(data, &ignored);
    printf("after  ropjit_run\n");

    return 0;
}
