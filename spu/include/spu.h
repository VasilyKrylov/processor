#ifndef K_SPU_H
#define K_SPU_H

#include "stack.h"
#include "stack.h"

enum spuErrors 
{
    SPU_OK                  = 0,
    SPU_BYTECODE_NULL       = 1 << 0,
    SPU_BYTECODE_OVERFLOW   = 1 << 1
};

enum runtimeErrors
{
    RE_OK                              = 0,
    RE_MISSING_ARGUMENT                = 1 << 0,
    RE_NOT_ENOGUH_ELEMENTS_IN_STACK    = 1 << 1,
    RE_DIVISION_BY_ZERO                = 1 << 2,
    RE_SQRT_NEGATIVE_ARGUMENT          = 1 << 3,
    RE_INVALID_INPUT                   = 1 << 4
}; // TODO: SpuErrorPrint

#ifdef PRINT_DEBUG
    struct spuVarInfo_t 
    {
        const char *name = NULL;
        const char *file = NULL;
        int line         = 0;
        const char *func = NULL;

        int error        = SPU_OK;
    };

    #define SPU_DUMP(spuName, comment) SpuDump (spuName, comment, __FILE__, __LINE__, __func__);

    #define SPU_CTOR(spuName, size) SpuCtor (spuName, size,                              \
                                                        spuVarInfo_t{.name = #spuName,           \
                                                                     .file = __FILE__,             \
                                                                     .line = __LINE__,             \
                                                                     .func = __func__});
    #define SPU_ERROR(stack) SpuError (stack);  
#else
    #define SPU_DUMP(stack, comment) 
    #define SPU_CTOR(spuName, size) SpuCtor (spuName, size);
    #define SPU_ERROR(stack) OK;
#endif // PRINT_DEBUG

struct spu_t
{
    stack_t stack;

    int *bytecode = NULL;
    size_t bytecodeCnt = 0;
    
    size_t ip = 0; 
    int regs[8] = {0}; // RAX, RBX, RCX, RDX
#ifdef PRINT_DEBUG
    spuVarInfo_t varInfo = {};
#endif // PRINT_DEBUG
};

#ifdef PRINT_DEBUG
    #define SPU_VERIFY(stack) SpuVerify (stack);  
#else
    #define SPU_VERIFY(stack) SPU_OK
#endif //PRINT_DEBUG

// TODO: macro for SpuCtor

int SpuCtor (spu_t *spu, char *inputFileName
             ON_DEBUG(, spuVarInfo_t varInfo));
int SpuDtor (spu_t *spu);
int SpuRun (spu_t *spu);
int SpuVerify (spu_t *spu);
void RuntimePrintError (int error); // TODO: does I really need this function ?
void SpuDump (spu_t *spu, const char *comment,
              const char *file, int line, const char * func);

#endif // K_SPU_H