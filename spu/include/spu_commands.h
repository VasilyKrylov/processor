#ifndef K_SPU_COMMANDS_H
#define K_SPU_COMMANDS_H

#include "spu.h"

#define CMP_BELOW               <
#define CMP_BELOW_OR_EQUAL      <=
#define CMP_ABOVE               >
#define CMP_ABOVE_OR_EQUAL      >=
#define CMP_EQUAL               ==
#define CMP_NOT_EQUAL           !=

int GetOperands (spu_t *spu, stackDataType *first, stackDataType *second);

int DoPush  (spu_t *spu);
int DoPop   (spu_t *spu);
int DoAdd   (spu_t *spu);
int DoSub   (spu_t *spu);
int DoDiv   (spu_t *spu);
int DoMul   (spu_t *spu);
int DoSqrt  (spu_t *spu);
int DoOut   (spu_t *spu);
int DoIn    (spu_t *spu);
int DoPushr (spu_t *spu);
int DoPopr  (spu_t *spu);
int DoJmp   (spu_t *spu);
int DoJb    (spu_t *spu);
int DoJbe   (spu_t *spu);
int DoJa    (spu_t *spu);
int DoJae   (spu_t *spu);
int DoJe    (spu_t *spu);
int DoJne   (spu_t *spu);
int DoCall  (spu_t *spu);
int DoRet   (spu_t *spu);
int DoPushm (spu_t *spu);
int DoPopm  (spu_t *spu);
int DoDraw  (spu_t *spu);

struct command_function 
{
    commandsBytecode bytecode = SPU_EMPTY;
    spuAction_t spuFunction = NULL;
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

#endif // K_SPU_COMMANDS