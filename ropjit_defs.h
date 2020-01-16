#include <stddef.h>
#include <stdint.h>

typedef enum {
    ROPJIT_REG_RAX,
    ROPJIT_REG_RCX,
    ROPJIT_REG_RDX,
    ROPJIT_REG_RBX,
    ROPJIT_REG_RBP,
    ROPJIT_REG_RSI,
    ROPJIT_REG_RDI,
    ROPJIT_REG_RSP,
    ROPJIT_REG_R8,
    ROPJIT_REG_R9,
    ROPJIT_REG_R10,
    ROPJIT_REG_R11,
    ROPJIT_REG_R12,
    ROPJIT_REG_R13,
    ROPJIT_REG_R14,
    ROPJIT_REG_R15
} ropjit_reg_t;

typedef void (*ropjit_gadget_t)();
