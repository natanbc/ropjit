#include "ropjit.h"
#include <stdio.h>
#include <sys/syscall.h> //for SYS_x defs

// syscall args (syscall number goes into rax)
// x86-64        rdi   rsi   rdx   r10   r8    r9    -
// SYS_write
// ssize_t write(int fd, const void *buf, size_t count);

int main(void) {
    uint64_t data[10000];
    data[0] = (uint64_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[1] = SYS_write;
    data[2] = (uint64_t)ropjit_gadget_load_immediate(ROPJIT_REG_RSI);
    data[3] = (uint64_t)"Hello World\n";
    data[4] = (uint64_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDX);
    data[5] = 12; // strlen("Hello World\n");
    data[6] = (uint64_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDI);
    data[7] = 1;
    data[8] = (uint64_t)ropjit_syscall;
    data[9] = (uint64_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[10] = 0; //dummy, will be replaced in ropjit_run
    data[11] = (uint64_t)ropjit_gadget_leavejit(ROPJIT_REG_RAX);

    printf("before ropjit_run\n");
    ropjit_run(data, &data[10]);
    printf("after  ropjit_run\n");

    return 0;
}
