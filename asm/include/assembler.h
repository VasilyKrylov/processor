#ifndef K_ASSEMBLER_H
#define K_ASSEMBLER_H

#include "file.h"

struct asm_t
{
    text_t text = {}; // TODO: should it be a pointer?

    int *bytecode = NULL;
    size_t instructionsCnt = 0; // TODO: maybe numberOfOperands or other name?
    // TODO: save file name here maybe
};

enum assemblerErrors
{
    ASSEMBLER_OK                  = 0,
    ASSEMBLER_UKNOWN_COMMAND      = 1 << 0,
    ASSEMBLER_MISSING_ARGUMENT    = 1 << 1,
    ASSEMBLER_EXTRA_ARGUMENT      = 1 << 2
};

int AsmCtorAndRead (char *inputFileName, asm_t *myAsm);
int Assemble (asm_t *myAsm);
int PrintBytecode (const char *outputFileName, asm_t *myAsm);
int AsmDtor (asm_t *myAsm);

#endif // K_ASSEMBLER_H