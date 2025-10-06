#ifndef K_SPU_COMMANDS_H
#define K_SPU_COMMANDS_H

#include "spu.h"

int DoPush (spu_t *processor);
int DoPop (spu_t *processor);
int DoAdd (spu_t *processor);
int DoSub (spu_t *processor);
int DoDiv (spu_t *processor);
int DoMul (spu_t *processor);
int DoSqrt (spu_t *processor);
int DoOut (spu_t *processor);
int DoIn (spu_t *processor);

#endif // K_SPU_COMMANDS