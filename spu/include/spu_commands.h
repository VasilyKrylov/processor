#ifndef K_SPU_COMMANDS_H
#define K_SPU_COMMANDS_H

#include "spu.h"

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

#endif // K_SPU_COMMANDS