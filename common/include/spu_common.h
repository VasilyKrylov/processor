#ifndef K_SPU_COMMON_H
#define K_SPU_COMMON_H

enum commandsBytecode
{
    SPU_PUSH    =  1,
    SPU_POP     =  2,
    SPU_ADD     =  3,
    SPU_SUB     =  4,
    SPU_DIV     =  5,
    SPU_MUL     =  6,
    SPU_SQRT    =  7,
    SPU_OUT     =  8,
    SPU_IN      =  9,
//      JUMP family
    SPU_JMP     =  10,
    SPU_JB      =  11, // <
    SPU_JBE     =  12, // <=
    SPU_JA      =  13, // >
    SPU_JAE     =  14, // >=
    SPU_JE      =  15, // ==
    SPU_JNE     =  16, // !=
//
    SPU_CALL    =  17,
    SPU_RET     =  18,

    SPU_PUSHM   =  19,
    SPU_POPM    =  20,
    SPU_DRAW    =  21,

    SPU_PUSHR   =  35,
    SPU_POPR    =  42,
    SPU_HLT     = -1,
}; 

const size_t NUMBER_OF_REGISTERS = 8;
const size_t REGISTER_NAME_LEN = 3;
const size_t SHIFT_REGISTER = 5;

const size_t MY_ASM_VERSION = 1337+1;

#endif // K_SPU_COMMON_H
