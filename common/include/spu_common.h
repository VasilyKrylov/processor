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
    SPU_JMP    =  10,
    SPU_PUSHR   =  35,
    SPU_POPR    =  42,
    SPU_HLT     = -1,
}; 

#endif // K_SPU_COMMON_H
