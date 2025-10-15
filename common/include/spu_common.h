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
    SUP_SQRT    =  7,
    SPU_OUT     =  8,
    SPU_IN      =  9,
//      JUMP family
    SPU_JMP    =  64+10,
    SPU_JB     =  64+11, // <
    SPU_JBE    =  64+12, // <=
    SPU_JA     =  64+13, // >
    SPU_JAE    =  64+14, // >=
    SPU_JE     =  64+15, // ==
    SPU_JNE    =  64+16, // !=
//
    SPU_PUSHR   =  35,
    SPU_POPR    =  42,
    SPU_HLT     = -1,
}; 

const size_t NUMBER_OF_REGISTERS = 8;
const size_t MAX_COMMAND_LEN = 8;

#endif // K_SPU_COMMON_H
