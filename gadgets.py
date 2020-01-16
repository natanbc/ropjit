#!/usr/bin/python3

import itertools

# move function_name to external func, replace with basename

def function_name(base, regs):
    return "ropjit_" + (
            base if (len(regs) == 0 or regs == [""]) else (base + '_' + '_'.join(regs))
    )

def reg_constant(name):
    return "ROPJIT_REG_" + name.upper()

class LoadImmediate:
    def is_valid(regs):
        return regs[0] != "rsp"

    def base_name():
        return "load_immediate"
    def codegen(r):
        size = "qword"
        byte_size = 8
        return f"""
    mov {r}, {size} ptr [rsp]
    add rsp, {byte_size}
    ret
    """

class Move:
    def is_valid(regs):
        return regs[0] != regs[1]

    def base_name():
        return "move"
    def codegen(a, b):
        return f"""
    mov {a}, {b}
    ret
    """

class Load:
    def base_name():
        return "load"
    def codegen(a, b):
        return f"""
    mov {a}, qword ptr [{b}]
    ret
    """

class Store:
    def base_name():
        return "store"
    def codegen(a, b):
        return f"""
    mov qword ptr [{a}], {b}
    ret
    """

class Debugger:
    def base_name():
        return "debugger"
    def codegen(_a):
        return """
    int3
    ret
    """

class Nop:
    def base_name():
        return "nop"
    def codegen(_a):
        return """
    ret
    """

class Syscall:
    def base_name():
        return "syscall"
    def codegen(_a):
        return """
    syscall
    ret
    """

class Call:
    def base_name():
        return "call"
    def codegen(a):
        return f"""
    call {a}
    ret
    """

class Add:
    def base_name():
        return "add"
    def codegen(a, b):
        return f"""
    add {a}, {b}
    ret
    """

class Sub:
    def base_name():
        return "sub"
    def codegen(a, b):
        return f"""
    sub {a}, {b}
    ret
    """

class Mul:
    def base_name():
        return "mul"
    def codegen(a, b):
        return f"""
    imul {a}, {b}
    ret
    """

#class Div:
#    def base_name():
#        return "div"
#    def codegen(a, b):
#        return f"""
#    idiv {a}, {b}
#    ret
#    """

class Jump:
    def base_name():
        return "jump"
    def codegen(a):
        return f"""
    jmp {a}
    """

class CopyMemory:
    def base_name():
        return "copy_memory"
    def codegen(_a):
        return f"""
    cld
    rep movsb
    ret
    """

class Registers:
    def __init__(self, regs, count = 1):
        self.__regs = regs
        self.__count = count

    def all(self):
        return self.__regs

    def no_args(self):
        return self.__count < 1

    def arg_count(self):
        return self.__count

    def combinations(self):
        if self.__count > 1:
            return itertools.product(self.__regs, repeat = self.__count)
        else:
            return self.__regs

general_purpose = [
        "rax", "rcx", "rdx", "rbx", "rbp", "rsi", "rdi", "rsp",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
]
gen = [
        (LoadImmediate, Registers(general_purpose)),
        (Move,          Registers(general_purpose, count = 2)),
        (Load,          Registers(general_purpose, count = 2)),
        (Store,         Registers(general_purpose, count = 2)),
        (Debugger,      Registers([""], count = 0)),
        (Nop,           Registers([""], count = 0)),
        (Syscall,       Registers([""], count = 0)),
        (Call,          Registers(general_purpose)),
        (Add,           Registers(general_purpose, count = 2)),
        (Sub,           Registers(general_purpose, count = 2)),
        (Mul,           Registers(general_purpose, count = 2)),
#        (Div,           Registers(general_purpose, count = 2)),
        (Jump,          Registers(general_purpose)),
        (CopyMemory,    Registers([""], count = 0)),
]

def gen_assembly(generator, regs):
    name = function_name(generator.base_name(), regs);
    return f"""
{name}: .globl {name}
{generator.codegen(*regs).lstrip()}"""

def gen_declaration(generator, regs):
    name = function_name(generator.base_name(), regs);
    return f"void {name}();"

def gen_switch_declaration(generator, regs, semicolon = True):
    if regs.no_args(): 
        return f"/* {generator.base_name()} takes no args, no switch needed */"
    args = ", ".join([f"ropjit_reg_t r{i}" for i in range(regs.arg_count())])
    s = ";" if semicolon else ""
    return f"""
ropjit_gadget_t ropjit_gadget_{generator.base_name()}(
    {args}
){s}"""

def gen_switch(generator, regs):
    definition = gen_switch_declaration(generator, regs, semicolon = False);
    if regs.no_args():
        return definition
    res = [definition]
    res.append("{")
    
    def gen_level(prefix, indent_level = 0):
        def indented(s):
            res.append("    " * (len(prefix) + 1 + indent_level) + s)

        if len(prefix) < regs.arg_count():
            indented(f"switch(r{len(prefix)}) {{")
            for r in regs.all():
                indent_level += 1
                indented(f"case {reg_constant(r)}: {{")
                gen_level(prefix + [r], indent_level = indent_level)
                indented("} break;")
                indent_level -= 1
            indented("}")
            return
        if hasattr(generator, "is_valid"):
            if not generator.is_valid(prefix):
                indented("return NULL;")
                return
        indented(f"return {function_name(generator.base_name(), prefix)};")

    gen_level([])

    res.append("    return NULL;")
    res.append("}")
    return '\n'.join(res)

def generate(header, how, write):
    write(header)
    for f, reg_list in gen:
        for regs in reg_list.combinations():
            if isinstance(regs, tuple):
                regs = list(regs)
            if not isinstance(regs, list):
                regs = [regs]

            if hasattr(f, "is_valid"):
                if not f.is_valid(regs):
                    continue
            
            write(how(f, regs))

def generate_switch(f, write):
    for g, reg_list in gen:
        write(f(g, reg_list))

with open("generated/gadgets.s", "w") as f:
    generate(".intel_syntax noprefix\n", gen_assembly, lambda x: f.write(x + "\n"))

with open("generated/gadgets.h", "w") as f:
    includes = """
#include "ropjit_defs.h"
"""
    generate(includes, gen_declaration, lambda x: f.write(x + "\n"))
    f.write("\n\n\n")
    generate_switch(gen_switch_declaration, lambda x: f.write(x + "\n"))

with open("generated/gadgets.c", "w") as f:
    f.write("""
#include "gadgets.h"

""")
    generate_switch(gen_switch, lambda x: f.write(x + "\n"))
