#ifndef K_SPU_H
#define K_SPU_H

#include "stack.h"

struct spu_t
{
    stack_t stack;

    int *bytecode = NULL;
    size_t bytecodeCnt = 0;
    
    size_t ip = 0; 
    int regs[8] = {0}; // RAX, RBX, RCX, RDX
};

enum spuErrors 
{
    SPU_OK                  = 0,
    SPU_BYTECODE_NULL       = 1 << 0,
    SPU_BYTECODE_OVERFLOW   = 1 << 1
};

#ifdef PRINT_DEBUG
    #define SPU_VERIFY(stack) SpuVerify (stack);  
#else
    #define SPU_VERIFY(stack) SPU_OK
#endif //PRINT_DEBUG

enum runtimeErrors
{
    RE_OK                              = 0,
    RE_MISSING_ARGUMENT                = 1 << 0,
    RE_NOT_ENOGUH_ELEMENTS_IN_STACK    = 1 << 1,
    RE_DIVISION_BY_ZERO                = 1 << 2,
    RE_SQRT_NEGATIVE_ARGUMENT          = 1 << 3,
    RE_INVALID_INPUT                   = 1 << 4
}; // TODO: SpuErrorPrint

// TODO: macro for SpuCtor

int SpuCtor (spu_t *spu, char *inputFileName);
int SpuDtor (spu_t *spu);
int SpuRun (spu_t *spu);
int SpuVerify (spu_t *spu);
void RuntimePrintError (int error); // TODO: does I really need this function ?


#endif // K_SPU_H