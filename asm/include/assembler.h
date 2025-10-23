#ifndef K_ASSEMBLER_H
#define K_ASSEMBLER_H

#include "file.h"
#include "spu_common.h"

const int LABEL_DEFAULT_VALUE = -1;
const int ARGUMENT_DEFAULT_VALUE = -1337;

struct label_t
{
    unsigned long hash = 0;
    ssize_t value      = 0;
};
struct asm_t
{
    text_t text = {};

    int *bytecode = NULL;

    size_t ip = 0;
    size_t instructionsCnt = 0;

    char *fileName = NULL;
    size_t lineNumber = 0; // FIXME: use this in code

    label_t labels[SPU_MAX_LABELS_COUNT] = {};
    size_t labelIdx = 0;
    
    FILE *fileListing = NULL;
};

enum assemblerErrors
{
    MY_ASM_OK                                = 0,
    MY_ASM_UKNOWN_COMMAND                    = 1 << 0,
    MY_ASM_MISSING_ARGUMENT                  = 1 << 1,
    MY_ASM_WRONG_ARGUMENT_TYPE               = 1 << 2,
    MY_ASM_TRASH_SYMBOLS                     = 1 << 3,
    MY_ASM_INVALID_REGISTER_NAME             = 1 << 4,
    MY_ASM_INVALID_REGISTER_ADDRESS_NAME     = 1 << 4,
    MY_ASM_LABEL_DOUBLE_ASSIGNMENT           = 1 << 5,
    MY_ASM_BAD_LABEL                         = 1 << 6,
    MY_ASM_TRASH_AFTER_COMMAND               = 1 << 7,


    MY_ASM_COMMON_ERROR              = 1 << 31
};

struct command_t 
{
    const char *name = NULL;
    int bytecode = 0;
    int argType = 0;
    size_t nameLen = 0;
};
enum argumentType
{
    MY_ASM_ARG_EMPTY            = 0,
    MY_ASM_ARG_NUMBER           = 1 << 0,
    MY_ASM_ARG_LABEL            = 1 << 1,
    MY_ASM_ARG_REGISTER         = 1 << 2,
    MY_ASM_ARG_REGISTER_ADDRESS = 1 << 3,

    MY_ASM_ARG_UKNOWN           = 1 << 31
};
enum asmPass_t
{
    MY_ASM_FIRST_PASS = 1,
    MY_ASM_FINAL_PASS = 2
};

#define SPU_COMMAND(commandName, argumentEnum) {.name = #commandName,           \
                                                .bytecode =  SPU_##commandName, \
                                                .argType = argumentEnum,        \
                                                .nameLen = sizeof(#commandName)}
const command_t commands[] = 
{
    SPU_COMMAND (PUSH,  MY_ASM_ARG_NUMBER           ),
    SPU_COMMAND (POP,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (ADD,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (SUB,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (DIV,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (MUL,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (SQRT,  MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (OUT,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (IN,    MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (JMP,   MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (JB,    MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (JBE,   MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (JA,    MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (JAE,   MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (JE,    MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (JNE,   MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (CALL,  MY_ASM_ARG_LABEL            ),
    SPU_COMMAND (RET,   MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (PUSHM, MY_ASM_ARG_REGISTER_ADDRESS ),
    SPU_COMMAND (POPM,  MY_ASM_ARG_REGISTER_ADDRESS ),
    SPU_COMMAND (PUSHR, MY_ASM_ARG_REGISTER         ),
    SPU_COMMAND (POPR,  MY_ASM_ARG_REGISTER         ),
    SPU_COMMAND (DRAW,  MY_ASM_ARG_EMPTY            ),
    SPU_COMMAND (HLT,   MY_ASM_ARG_EMPTY            )
};
const size_t CommandsNumber = sizeof(commands) / sizeof(command_t);
#undef SPU_COMMAND

struct argument_t
{
    argumentType type = MY_ASM_ARG_UKNOWN;
    int value = ARGUMENT_DEFAULT_VALUE;
};

int AllocateBytecode            (asm_t *myAsm);
int AsmCtor                     (asm_t *myAsm);
int AsmRead                     (asm_t *myAsm, char *inputFileName);
int Assemble                    (asm_t *myAsm);
int PrintBinary                 (const char *outputFileName, asm_t *myAsm);
int AsmPrintError               (int error);
int AsmDtor                     (asm_t *myAsm);

#endif // K_ASSEMBLER_H