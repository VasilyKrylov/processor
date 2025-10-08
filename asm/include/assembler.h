#ifndef K_ASSEMBLER_H
#define K_ASSEMBLER_H

#include "file.h"

struct asm_t
{
    text_t text = {};

    int *bytecode = NULL;

    size_t ip = 0;
    size_t instructionsCnt = 0;
    char *fileName = NULL;
    // TODO: line number here
};

enum assemblerErrors
{
    MY_ASM_OK                        = 0,
    MY_ASM_UKNOWN_COMMAND            = 1 << 0,
    MY_ASM_MISSING_ARGUMENT          = 1 << 1,
    MY_ASM_EXTRA_ARGUMENT            = 1 << 2,
    MY_ASM_INVALID_REGISTER_NAME     = 1 << 3,
    


    MY_ASM_COMMON_ERROR              = 1 << 31
};

int AsmRead (asm_t *myAsm, char *inputFileName);
int Assemble (asm_t *myAsm);
int PrintBytecode (const char *outputFileName, asm_t *myAsm);
int AsmPrintError (int error);
int AsmDtor (asm_t *myAsm);

#endif // K_ASSEMBLER_H