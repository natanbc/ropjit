#include "ropjit.h"
#include <stdio.h>       //printf
#include <stdlib.h>      //malloc
#include <string.h>      //memcpy

void smash_it_again() {
    printf("smash_it_again\n");
}

void do_it_some_more() {
    printf("do_it_some_more\n");
}

void smash_the_stack() {
    smash_it_again();
    do_it_some_more();
}

int main(void) {
    uintptr_t* real_code;
    uintptr_t data[40];

    //call smash_the_stack(), overwrites and corrupts the first 3 values
    //call instruction already breaks data[2], data[1] and data[0] can't
    //be trusted since they'd be used by the called function if it touches
    //the stack.
    data[0] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[1] = (uintptr_t)smash_the_stack;
    data[2] = (uintptr_t)ropjit_gadget_call(ROPJIT_REG_RAX);

    //prepare to overwrite the "corrupted" data back with the original state,
    //so the loop can be restarted
    //ropjit_copy_memory copies RCX bytes from the address at RSI to the
    //address at RDI, decrementing RCX and incrementing both RSI and RDI.
    //These registres are hardcoded in the movsb instruction, so they can't
    //be changed without storing the old values somewhere and then restoring them.
    data[3] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RSI);
    data[4] = (uintptr_t)data;
    data[5] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RDI);
    data[6] = (uintptr_t)&real_code;
    //since the malloc'd address isn't known yet (in this example malloc could be called
    //earlier, but if the size of the code isn't known ahead of time something like this
    //would be needed), a pointer to where the address will be stored is hardcoded, then
    //it can be dereferenced to obtain the actual address
    data[7] = (uintptr_t)ropjit_gadget_load(ROPJIT_REG_RDI, ROPJIT_REG_RDI);

    //data[0..2] need to be restored, so 3 * uintptr_t values have to be copied
    data[8] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RCX);
    data[9] =           3 /* 0, 1, 2 */ * sizeof(uintptr_t);

    //this could be any register other than RCX, RDI and RSI
    data[10] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[11] = (uintptr_t)ropjit_copy_memory;
    //"call it", abusing the ret in ropjit_copy_memory to run the next instruction
    data[12] = (uintptr_t)ropjit_gadget_jump(ROPJIT_REG_RAX);

    //load the address again. Since RDI is still valid, it could just be subtracted,
    //so the commented code would also work
    /*
    data[13] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[14] =           3 * sizeof(uintptr_t);
    data[15] = (uintptr_t)ropjit_gadget_sub(ROPJIT_REG_RDI, ROPJIT_REG_RAX);
    data[16] = (uintptr_t)ropjit_gadget_move(ROPJIT_REG_RSP, ROPJIT_REG_RDI);
    */
    data[13] = (uintptr_t)ropjit_gadget_load_immediate(ROPJIT_REG_RAX);
    data[14] = (uintptr_t)&real_code;
    data[15] = (uintptr_t)ropjit_gadget_load(ROPJIT_REG_RAX, ROPJIT_REG_RAX);
    data[16] = (uintptr_t)ropjit_gadget_move(ROPJIT_REG_RSP, ROPJIT_REG_RAX);
    //jump is not needed as the move already causes a jump, since
    //it's
    //  mov rsp, rax
    //  ret
    
    //allocate a stack
    uintptr_t* mem = malloc(sizeof(uint64_t) * 1000);
    //use only the last bytes for the "code"
    real_code = &mem[980];
    memcpy(real_code, data, 20 * sizeof(uintptr_t));

    printf("original: %p, executed: %p\n", data, real_code);
    printf("before ropjit_run\n");
    uintptr_t ignored; //infinite loop, no need for the return address
    ropjit_run(real_code, &ignored);
    printf("after  ropjit_run\n");

    return 0;
}
