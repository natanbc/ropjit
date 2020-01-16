# ropjit

Abuse `ret` instructions to build ROPchains for running arbitrary code. This
can be used to JIT code on platforms that don't allow creating arbitrary
executable memory (eg iOS), because all the code has already been created
at compile-time, only creating a fake stack at runtime.

# How

The `gadgets.py` script generates the gadgets, their declarations and the
helper functions that select which gadget to use based on the registers wanted
(eg `ropjit_gadget_move(ROPJIT_REG_RAX, ROPJIT_REG_RDI)` returns
`ropjit_move_rax_rdi`).

By building a ROPchain with these gadgets, arbitrary code can be executed.

# Why

I was bored and had nothing better to do.

# Getting Started

- `make run`/`make run EXAMPLE=loop` to run the examples

The `loop` example should run in any x86-64 machine, the `helloworld`
example only runs on linux, since it does syscalls manually.


