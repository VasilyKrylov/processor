#ifndef K_ASSEMBLER_H
#define K_ASSEMBLER_H

#include "file.h"

const int LABEL_DEFAULT_VALUE = -1;

struct asm_t
{
    text_t text = {};

    int *bytecode = NULL;

    size_t ip = 0;
    size_t instructionsCnt = 0;

    char *fileName = NULL;
    size_t lineNumber = 0; // FIXME: use this in code

    ssize_t labels[10] = {};

    FILE *fileListing = NULL;
};

enum assemblerErrors
{
    MY_ASM_OK                        = 0,
    MY_ASM_UKNOWN_COMMAND            = 1 << 0,
    MY_ASM_MISSING_ARGUMENT          = 1 << 1,
    MY_ASM_TRASH_SYMBOLS             = 1 << 2,
    MY_ASM_INVALID_REGISTER_NAME     = 1 << 3,
    MY_ASM_LABEL_DOUBLE_ASSIGNMENT   = 1 << 4,
    MY_ASM_BAD_LABEL_NAME            = 1 << 5,
    MY_ASM_TRASH_AFTER_COMMAND       = 1 << 5,


    MY_ASM_COMMON_ERROR              = 1 << 31
};

int AsmCtor (asm_t *myAsm);
int AsmRead (asm_t *myAsm, char *inputFileName);
int Assemble (asm_t *myAsm);
int PrintBytecode (const char *outputFileName, asm_t *myAsm);
int AsmPrintError (int error);
int AsmDtor (asm_t *myAsm);

#endif // K_ASSEMBLER_H