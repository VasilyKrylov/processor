#ifndef K_SPU_H
#define K_SPU_H

#include "stack.h"
#include "spu_common.h"

const size_t SPU_DUMP_BYTECODE_ROW_LEN = 8;
const size_t SPU_VIDEO_WIDTH           = 50;
const size_t SPU_VIDEO_HEIGHT          = 20;
const size_t SPU_MAX_RAM_SIZE          = SPU_VIDEO_WIDTH * SPU_VIDEO_HEIGHT;


enum spuErrors 
{
    SPU_OK                  = 0,
    SPU_NULL_STRUCT         = 1 << 0,
    SPU_BYTECODE_NULL       = 1 << 1,
    SPU_BYTECODE_OVERFLOW   = 1 << 2,
    SPU_WRONG_VERSION       = 1 << 3,

    SPU_COMMON_ERROR        = 1 << 31
};

enum runtimeErrors
{
    RE_OK                              = 0,
    RE_MISSING_ARGUMENT                = 1 << 0,
    RE_NOT_ENOGUH_ELEMENTS_ON_STACK    = 1 << 1,
    RE_DIVISION_BY_ZERO                = 1 << 2,
    RE_SQRT_NEGATIVE_ARGUMENT          = 1 << 3,
    RE_INVALID_INPUT                   = 1 << 4,
    RE_UKNOWN_BYTECODE                 = 1 << 5,
    RE_JMP_ARGUMENT_IS_NEGATIVE        = 1 << 6, // TODO: add print for this error code
    RE_CALL_ARGUMENT_IS_NEGATIVE       = 1 << 7, 
    RE_RET_VALUE_IS_NEGATIVE           = 1 << 8
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

    #define SPU_CTOR(spuName) SpuCtor (spuName,                              \
                                        spuVarInfo_t{.name = #spuName,           \
                                                        .file = __FILE__,             \
                                                        .line = __LINE__,             \
                                                        .func = __func__});
    #define SPU_ERROR(spu) SpuError (spu);  
#else
    #define SPU_DUMP(spu, comment) // TODO: why empty
    #define SPU_CTOR(spuName) SpuCtor (spuName);
    #define SPU_ERROR(spu) SPU_OK;
#endif // PRINT_DEBUG

struct spu_t
{
    stack_t stack;
    stack_t stackReturn;

    int *bytecode;
    size_t bytecodeCnt;
    
    size_t ip; 
    int regs[NUMBER_OF_REGISTERS] = {0}; // RAX, RBX, RCX, RDX

    int ram[SPU_MAX_RAM_SIZE] = {0};

#ifdef PRINT_DEBUG
    spuVarInfo_t varInfo = {};
#endif // PRINT_DEBUG
};

#ifdef PRINT_DEBUG
    #define SPU_VERIFY(stack) SpuVerify (stack);  
#else
    #define SPU_VERIFY(stack) SPU_OK
#endif //PRINT_DEBUG

#include "spu_commands.h"
struct command_function 
{
    commandsBytecode bytecode = SPU_EMPTY;
    int (*spuFunction) (spu_t *spu) = NULL;
};

// #define COMMAND_FUNCTION (commandName) {.bytecode = SPU_##commandName, 
//                                         .spuFunction = Do##commandName}

const command_function commandFunctions[] = 
{
    {.bytecode = SPU_PUSH,  .spuFunction = DoPush   },
    {.bytecode = SPU_POP,   .spuFunction = DoPop    },
    {.bytecode = SPU_ADD,   .spuFunction = DoAdd    },
    {.bytecode = SPU_SUB,   .spuFunction = DoSub    },
    {.bytecode = SPU_DIV,   .spuFunction = DoDiv    },
    {.bytecode = SPU_MUL,   .spuFunction = DoMul    },
    {.bytecode = SPU_SQRT,  .spuFunction = DoSqrt   },
    {.bytecode = SPU_OUT,   .spuFunction = DoOut    },
    {.bytecode = SPU_IN,    .spuFunction = DoIn     },
    {.bytecode = SPU_JMP,   .spuFunction = DoJmp    },
    {.bytecode = SPU_JB,    .spuFunction = DoJb     },
    {.bytecode = SPU_JBE,   .spuFunction = DoJbe    }, 
    {.bytecode = SPU_JA,    .spuFunction = DoJa     },
    {.bytecode = SPU_JAE,   .spuFunction = DoJae    },
    {.bytecode = SPU_JE,    .spuFunction = DoJe     },
    {.bytecode = SPU_JNE,   .spuFunction = DoJne    },
    {.bytecode = SPU_CALL,  .spuFunction = DoCall   },
    {.bytecode = SPU_RET,   .spuFunction = DoRet    },
    {.bytecode = SPU_PUSHM, .spuFunction = DoPushm  },
    {.bytecode = SPU_POPM,  .spuFunction = DoPopm   },
    {.bytecode = SPU_PUSHR, .spuFunction = DoPushr  },
    {.bytecode = SPU_POPR,  .spuFunction = DoPopr   },
    {.bytecode = SPU_DRAW,  .spuFunction = DoDraw   },
};
const size_t commandFunctionsLen = sizeof(commandFunctions) / sizeof (commandFunctions[0]);


int SpuCtor             (spu_t *spu
                         ON_DEBUG(, spuVarInfo_t varInfo));
int SpuRead             (spu_t *spu, char *inputFileName);
int SpuDtor             (spu_t *spu);
int SpuRun              (spu_t *spu);
int SpuVerify           (spu_t *spu);
void RuntimePrintError  (int error);
void SpuDump            (spu_t *spu, const char *comment,
                         const char *file, int line, const char * func);
int SpuError            (spu_t *spu);

#endif // K_SPU_H