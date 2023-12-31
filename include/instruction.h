#pragma once

#include <common.h>

typedef enum
{
    AM_NONE,
    AM_R_D16, // 16-bit immediate to register
    AM_R_R,   // Register to register
    AM_MR_R,  // Register to memory
    AM_R,     // Register
    AM_R_D8,
    AM_R_MR,
    AM_R_RI,
    AM_R_RD,
    AM_RI_R,
    AM_RD_R,
    AM_R_A8,
    AM_A8_R,
    AM_HL_SPR,
    AM_D16,
    AM_D8,
    AM_MR_D8,
    AM_MR,
    AM_A16_R,
    AM_R_A16,
} addr_mode;

typedef enum
{
    RT_NONE,
    RT_A,
    RT_F,
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_AF,
    RT_BC,
    RT_DE,
    RT_HL,
    RT_SP,
    RT_PC,
} reg_type;

typedef enum
{
    IN_NONE,
    IN_NOP,
    IN_LD,
    IN_INC,
    IN_DEC,
    IN_RLCA,
    IN_ADD,
    IN_RRCA,
    IN_STOP,
    IN_RLA,
    IN_JR,
    IN_RRA,
    IN_DAA,
    IN_CPL,
    IN_SCF,
    IN_CCF,
    IN_HALT,
    IN_ADC,
    IN_SUB,
    IN_SBC,
    IN_AND,
    IN_XOR,
    IN_OR,
    IN_CP,
    IN_POP,
    IN_JP,
    IN_PUSH,
    IN_RET,
    IN_CB,
    IN_CALL,
    IN_RETI,
    IN_LDH,
    IN_JPHL,
    IN_DI,
    IN_EI,
    IN_RST,
    IN_ERR,
    // CB instructions
    IN_RLC,
    IN_RRC,
    IN_RL,
    IN_RR,
    IN_SLA,
    IN_SRA,
    IN_SWAP,
    IN_SRL,
    IN_BIT,
    IN_RES,
    IN_SET,
} in_type;

typedef enum
{
    CT_NONE,
    CT_NZ,
    CT_Z,
    CT_NC,
    CT_C,
} cond_type;

typedef struct
{
    in_type type;
    addr_mode mode;
    reg_type reg_1, reg_2;
    cond_type cond;
    u8 param;
} instruction;

#ifdef __cplusplus
extern "C"
{
#endif

    instruction *instruction_by_opcode(u8 opcode);
    const char *instruction_name(instruction *);

#ifdef __cplusplus
}
#endif
