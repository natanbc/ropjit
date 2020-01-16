.intel_syntax noprefix

ropjit_switch:
    mov qword ptr [rsi], rsp
    mov rsp, rdi
    ret

ropjit_run: .globl ropjit_run
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    call ropjit_switch
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret
