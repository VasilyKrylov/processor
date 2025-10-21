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
int DoCall  (spu_t *spu);
int DoRet   (spu_t *spu);
int DoPushm (spu_t *spu);
int DoPopm  (spu_t *spu);
int DoDraw  (spu_t *spu);

#endif // K_SPU_COMMANDS