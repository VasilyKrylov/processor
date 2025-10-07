#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spu.h"

#include "spu_common.h"
#include "spu_commands.h"
#include "stack.h"
#include "debug.h"
#include "file.h"
#include "utils.h"

#include <math.h>

int SpuCtor (spu_t *spu, char *inputFileName
            ON_DEBUG(, spuVarInfo_t varInfo))
{
    STACK_CREATE (&spu->stack, 10)
#ifdef PRINT_DEBUG
    spu->varInfo = varInfo;
#endif // PRINT_DEBUG

    size_t bufferLen = 0;
    char *buffer = ReadFile (inputFileName, &bufferLen);

    if (buffer == NULL)
    {
        return COMMON_NULL_POINTER;
    }

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
            
            return status;
        }

        // DEBUG_PRINT ("regs[%lu] = {", sizeof(spu->regs) / sizeof(spu->regs[0]));
        // for (size_t i = 0; i < sizeof(spu->regs) / sizeof(spu->regs[0]); i++)
        // {
        //     DEBUG_PRINT ("%d ", spu->regs[i]);
        // }
        // DEBUG_PRINT ("%s", "}\n");

        SPU_DUMP (spu, "in switch case")
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
    if (error & RE_INVALID_INPUT)                  printf("%s", "Bro, input what you asked to...");

    printf("%s", COLOR_END);
}

void SpuDump (spu_t *spu, const char *comment,
              const char *file, int line, const char * func)
{
    printf ("spu [%p]", spu);
#ifdef PRINT_DEBUG
    printf (" \"%s\" %s:%d %s()", spu->varInfo.name,
                                 spu->varInfo.file,
                                 spu->varInfo.line,
                                 spu->varInfo.func);
#endif // PRINT_DEBUG
    puts ("");
    printf ("called at %s:%d %s()\n", file, line, func);
    printf ("reason: \"%s\"\n", comment);

    printf("%s", "{\n");
    printf("\tip          \t = %lu;\n", spu->ip);
    printf("\tbytecodeCnt \t = %lu;\n", spu->bytecodeCnt);
    
    printf("\tregs[%lu]   \t = [%p]\n", sizeof (spu->regs) / sizeof (spu->regs[0]), spu->regs);
    printf("%s", "\t{\n");
    for (size_t i = 0; i < sizeof (spu->regs) / sizeof (spu->regs[0]); i++)
    {
        printf("\t\tR%cX = %d;\n", char('A' + i), spu->regs[i]);
    }
    printf("%s", "\t}\n");
    
    printf("\tbytecode[%lu]   \t = [%p]\n", spu->bytecodeCnt, spu->bytecode);
    printf("%s", "\t{\n");
    for (size_t i = 0; i < spu->bytecodeCnt; i++)
    {
        printf ("%s", "\t\t");

        if (i == spu->ip)
            printf ("%s", "->");
        else 
            printf ("%s", "  ");
        
        printf("\t\t[%lu] = %d\n", i, spu->bytecode[i]); 
        // TODO: hex dump 
    }
    printf("%s", "\t}\n");

    
    printf("%s", "}\n");
    
    STACK_DUMP (spu->stack, comment)
}

int SpuError (spu_t *spu)
{
    int error = SPU_OK;

    if (spu == NULL)
    {
        error |= SPU_NULL_STRUCT;
        return error;
    }    
    if (spu->bytecode == NULL)
    {
        error |= SPU_BYTECODE_NULL;
        return error;
    }
    if (spu->ip > spu->bytecodeCnt)
    {
        error |= SPU_BYTECODE_OVERFLOW;
    }

    return error;
}
