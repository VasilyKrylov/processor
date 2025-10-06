#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "spu.h"

#include "spu_common.h"
#include "spu_commands.h"
#include "stack.h"
#include "debug.h"
#include "file.h"
#include "utils.h"

int SpuCtor (spu_t *spu, char *inputFileName)
{
    STACK_CREATE (&spu->stack, 10)

    size_t bufferLen = 0;
    char *buffer = ReadFile (inputFileName, &bufferLen);

    if (buffer == NULL)
    {
        return COMMON_NULL_POINTER;
    }
    // TODO: function from onegin what returns buffer
    // FILE *inputFile = fopen (inputFileName, "r");
    // if (inputFile == NULL)
    // {
    //     perror ("Error opening input file");

    //     return -1;
    // }

    // size_t fileSize = GetFileSize (inputFileName, );

    // char *buffer = (char *) calloc (fileSize + 1, sizeof(int));
    // if (buffer == NULL)
    // {
    //     perror ("Error allocating memory for buffer");

    //     return -1;
    // }
    
    // size_t readedChars = fread (buffer, sizeof(char), fileSize, inputFile);
    // if (readedChars < fileSize)
    // {
    //     perror ("Error while reading from file");

    //     return -1;
    // }
    // DEBUG_LOG ("readedChars: %lu", readedChars);
    // end of to do

    spu->bytecodeCnt = CountChrs (buffer, " ") + 1; // FIXME: CountChrs()
    spu->bytecode = (int *) calloc (spu->bytecodeCnt, sizeof(int));
    
    DEBUG_LOG ("spu->bytecodeCnt = %lu", spu->bytecodeCnt);

    const char *bytecodeDelimiter = " ";
    char *number = NULL;
    size_t idx = 0;

    number = strtok (buffer, bytecodeDelimiter);
    while (number != NULL)
    {
        // TODO: sscanf()
        spu->bytecode[idx] = atoi (number);
        
        DEBUG_LOG ("bytecode.data[%lu] = %d", idx, spu->bytecode[idx]);
        
        idx++;

        number = strtok (NULL, bytecodeDelimiter);
    }
    
    free (buffer);
    buffer = NULL;

    return RE_OK;
}

int SpuDtor (spu_t *spu)
{
    if (spu == NULL) return COMMON_NULL_POINTER;

    StackDtor (&spu->stack);

    free (spu->bytecode);
    spu->bytecode = NULL;

    spu->bytecodeCnt = 0;

    spu->ip = 0;

    memset (spu->regs, 0, sizeof(spu->regs));

    return COMMON_OK;
}

int SpuRun (spu_t *spu)
{
    if (spu == NULL) return COMMON_NULL_POINTER;

    DEBUG_LOG ("%s", "HELLO FRIENDS, TODAY WE WILL RUN SOFT PROCESSOR UNIT");
    DEBUG_LOG ("%s", "LET'S FUCKING GOOOOOOOOOOOOOOOOOOOOOOOOOO");

    for (; spu->ip < spu->bytecodeCnt; spu->ip++)
    {
        DEBUG_LOG ("processor->ip = %lu;", spu->ip); // MAKE SpuDump
        int status = 0;

        switch (spu->bytecode[spu->ip])
        {
            case SPU_PUSH:  status = DoPush  (spu); break;
            case SPU_POP:   status = DoPop   (spu); break;
            case SPU_ADD:   status = DoAdd   (spu); break;
            case SPU_SUB:   status = DoSub   (spu); break;
            case SPU_DIV:   status = DoDiv   (spu); break;
            case SPU_MUL:   status = DoMul   (spu); break;
            case SUP_SQRT:  status = DoSqrt  (spu); break;
            case SPU_OUT:   status = DoOut   (spu); break;
            case SPU_IN:    status = DoIn    (spu); break;
            case SPU_PUSHR: status = DoPushr (spu); break;
            case SPU_POPR:  status = DoPopr  (spu); break;
            case SPU_HLT:   return RE_OK;
            
            default:
                ERROR ("Uknown command in bytecode: %d", spu->bytecode[spu->ip]);
                
                return -1;
        }

        if (status != RE_OK)
        {
            RuntimePrintError (status);
        }

        DEBUG_PRINT ("regs[%lu] = {", sizeof(spu->regs) / sizeof(spu->regs[0]));
        for (size_t i = 0; i < sizeof(spu->regs) / sizeof(spu->regs[0]); i++)
        {
            DEBUG_PRINT ("%d ", spu->regs[i]);
        }
        DEBUG_PRINT ("%s", "}\n");

        STACK_DUMP (spu->stack, "in switch case")
        // TODO: new function: PROCESSOR_DUMP !!!!!!!!!!!!!!!

#ifdef PRINT_DEBUG
        getchar();
#endif // PRINT_DEBUG
    }

    return RE_OK;
}

// TODO: what is meaning of this function, if I print errors in spu_commands.cpp ?
void RuntimePrintError (int error)
{
    DEBUG_LOG ("error = %d\n", error);

    if (error == RE_OK) return;

    printf("%s", RED_BOLD_COLOR);

    // TODO: add missing argument for PUSH, POP and other?
    if (error & RE_MISSING_ARGUMENT)               printf("%s", "Missing argument for instruction!\n");
    if (error & RE_NOT_ENOGUH_ELEMENTS_IN_STACK)   printf("%s", "There is no enough elements on stack to run instruction!\n");
    if (error & RE_DIVISION_BY_ZERO)               printf("%s", "Bro, you can't divide by zero -_-\n");
    if (error & RE_SQRT_NEGATIVE_ARGUMENT)         printf("%s", "You can't calculate square root for negative value -_-\n");
    if (error & RE_INVALID_INPUT)         printf("%s", "Bro, input what you asked to...");

    printf("%s", COLOR_END);
}

int SpuVerify (spu_t *spu)
{
    int error = SPU_OK;

    if (spu->bytecode == NULL)              error |= SPU_BYTECODE_NULL;
    if (spu->ip > spu->bytecodeCnt) error |= SPU_BYTECODE_OVERFLOW;
    
    return error;
}