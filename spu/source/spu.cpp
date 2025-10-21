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

int (*functionPointers[SPU_MAX_COMMAND_VALUE + 1]) (spu_t *spu) = {NULL};

int SpuReadHeader   (spu_t *spu, char **buffer);
int SpuReadBytecode (spu_t *spu, char **buffer);

int SpuCtor (spu_t *spu
             ON_DEBUG(, spuVarInfo_t varInfo))
{
    int result = STACK_CREATE (&spu->stack, 8)
    result    |= STACK_CREATE (&spu->stackReturn, 8)

    if (result != STACK_OK) 
        return result; // NOTE: BUT HOW SOME FUNCTION IN MAIN WILL KNOW WHAT IT IS THE STACK ERROR, NOT SPU?
    
#ifdef PRINT_DEBUG
    spu->varInfo = varInfo;
#endif // PRINT_DEBUG

    spu->bytecode    = NULL;
    spu->bytecodeCnt = 0;
    spu->ip          = 0;

    for (size_t i = 0; i < commandFunctionsLen; i++)
    {
        functionPointers[commandFunctions[i].bytecode] = commandFunctions[i].spuFunction;
    }

    return SPU_OK;
}

// FIMXE: returning 0 and 1, should be enums
int SpuRead (spu_t *spu, char *inputFileName)
{
    size_t bufferLen = 0;
    char *const buffer = ReadFile (inputFileName, &bufferLen); // NOTE: is *const good?

    if (buffer == NULL)
    {
        return COMMON_ERROR_NULL_POINTER;
    }

    char *bufferPtr = buffer;

    int status = SpuReadHeader (spu, &bufferPtr);
    if (status != 0) 
    {
        free (buffer); // NOTE: this is local function, so I shouldn't do buffer=NULL, ok?
        
        return status;
    }

    status = SpuReadBytecode(spu, &bufferPtr);
    if (status != 0) 
    {
        free (buffer);

        return status;
    }

    free (buffer);

    return SPU_OK;
}

int SpuReadHeader (spu_t *spu, char **buffer)
{
    size_t version = 133722869;
    int bufferOffset = 0;

    int status = sscanf (*buffer, "%lu %n", &version, &bufferOffset);
    *buffer += bufferOffset;

    if (status == EOF) // TODO: make new function
    {
        ERROR_PRINT ("%s", "There is no version value in file...");

        return COMMON_ERROR_TO_EARLY_EOF;
    }
    if (status == 0)
    {
        ERROR_PRINT ("%s", "Error reading version from file...");

        return COMMON_ERROR_SSCANF;
    }

    if (version != MY_ASM_VERSION)
    {
        ERROR_PRINT ("Wrong version of binary. Required version is \"%lu\", but got \"%lu\"", 
                     MY_ASM_VERSION, version);

        return SPU_WRONG_VERSION;
    }

    status = sscanf (*buffer, "%lu %n", &spu->bytecodeCnt, &bufferOffset);
    *buffer += bufferOffset;

    if (status == EOF)
    {
        ERROR ("%s", "There is no version value in file...")

        return COMMON_ERROR_TO_EARLY_EOF;
    }
    if (status == 0)
    {
        ERROR ("%s", "Error reading version from file...")

        return COMMON_ERROR_SSCANF;
    }

    return SPU_OK;
}

int SpuReadBytecode (spu_t *spu, char **buffer)
{
    spu->bytecode = (int *) calloc (spu->bytecodeCnt, sizeof(int));
    if (spu->bytecode == NULL)
    {
        ERROR_PRINT ("%s", "Error allocating memory for bytecode");

        return SPU_COMMON_ERROR |
               COMMON_ERROR_ALLOCATING_MEMORY;
    }
    
    DEBUG_LOG ("spu->bytecodeCnt = %lu", spu->bytecodeCnt);

    char *bufferPtr = *buffer;
    for (size_t i = 0; i < spu->bytecodeCnt; i++)
    {
        int bufferOffset = 0;
        int status = sscanf (bufferPtr, "%d %n", &spu->bytecode[i], &bufferOffset);
        bufferPtr += bufferOffset;

        if (status == EOF)
        {
            ERROR ("There is %lu bytecodes in the input file ", i) // TODO: add file name in this error message
            ERROR ("Excepted %lu bytecodes from this file", spu->bytecodeCnt) 

            return 1; // FIXME:
        }
        if (status == 0)
        {
            ERROR ("Error reading bytecode from file \"%s\"."
                   "Bytecode with index [%lu] was incorrect", "FILE NAME HERE", i + 1)
            
            
            return 1; // TODO: one time it will be enum's...
        }
    }

    return RE_OK;
}

int SpuDtor (spu_t *spu)
{
    if (spu == NULL) return COMMON_ERROR_NULL_POINTER;

    StackDtor (&spu->stack);
    StackDtor (&spu->stackReturn);

    free (spu->bytecode);
    spu->bytecode = NULL;

    spu->bytecodeCnt = 0;

    spu->ip = 0;

    memset (spu->regs, 0, sizeof(spu->regs));

    return COMMON_OK;
}

// maybe not the best solution, but I think better than a lot of functions
#define CONDITION_JMP(spu, CMP)                                     \
{                                                                   \
    stackDataType first  = 0;                                       \
    stackDataType second = 0;                                       \
    status = GetOperands (spu, &first, &second);                    \
                                                                    \
    if (status == RE_OK)                                            \
    {                                                               \
        if (second CMP first)                                       \
            status = DoJmp (spu);                                   \
        else                                                        \
        {                                                           \
            spu->ip++;                                              \
            spu->ip++;                                              \
        }                                                           \
    }                                                               \
}

int SpuRun (spu_t *spu)
{
    if (spu == NULL) return COMMON_ERROR_NULL_POINTER;

    DEBUG_LOG ("%s", "HELLO FRIENDS, TODAY WE WILL RUN SOFT PROCESSOR UNIT");
    DEBUG_LOG ("%s", "LET'S GOOOOOOOOOOOOOOOOOOOOOOOOOO");

    for (; spu->ip < spu->bytecodeCnt ;)
    {
        SPU_DUMP (spu, "in switch case")

#ifdef PRINT_DEBUG
        getchar();
#endif // PRINT_DEBUG

        int bytecode = spu->bytecode[spu->ip];

        if (bytecode == -1)
            return RE_OK;

        int status = functionPointers[bytecode](spu);
        if (status != RE_OK)
        {
            // RuntimePrintError (status); 
            SPU_DUMP (spu, RED_BOLD_COLOR "Error occured..." COLOR_END);

            return status;
        }
    }

    return RE_OK;
}
#undef CONDITION_JMP

// TODO: what is meaning of this function, if I print errors in spu_commands.cpp ?
void RuntimePrintError (int error)
{
    DEBUG_LOG ("error = %d\n", error);

    if (error == RE_OK) return;

    printf("%s", RED_BOLD_COLOR);

    // TODO: add missing argument for PUSH, POP and other?
    if (error & RE_MISSING_ARGUMENT)               printf("%s", "Missing argument for instruction!\n");
    if (error & RE_NOT_ENOGUH_ELEMENTS_ON_STACK)   printf("%s", "There is no enough elements on stack to run instruction!\n");
    if (error & RE_DIVISION_BY_ZERO)               printf("%s", "Bro, you can't divide by zero -_-\n");
    if (error & RE_SQRT_NEGATIVE_ARGUMENT)         printf("%s", "You can't calculate square root for negative value -_-\n");
    if (error & RE_INVALID_INPUT)                  printf("%s", "Bro, please, input what you asked to...\n");
    if (error & RE_UKNOWN_BYTECODE)                printf("%s", "Uknown bytecode\n");

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
    
    // REGISTERS
    printf("\tregs[%lu]   \t = [%p]\n", sizeof (spu->regs) / sizeof (spu->regs[0]), spu->regs);
    printf("%s", "\t{\n");
    for (size_t i = 0; i < sizeof (spu->regs) / sizeof (spu->regs[0]); i++)
    {
        printf("\t\tR%cX = %d;\n", char('A' + i), spu->regs[i]);
    }
    printf("%s", "\t}\n");
    
    printf("\tbytecode[%lu]   \t = [%p]\n", spu->bytecodeCnt, spu->bytecode);
    printf("%s", "\t{");
    printf ("%s", "\n\t\t        ");
    for (size_t i = 0; i < SPU_DUMP_BYTECODE_ROW_LEN; i++)
        printf ("[%04lu] | ", i);

    for (size_t i = 0; i < spu->bytecodeCnt; i++)
    {
        if (i % SPU_DUMP_BYTECODE_ROW_LEN == 0) 
        {
            printf ("\n\t\t[%04lu]: ", i);
        }

        if (i == spu->ip)
            printf ("%s<%04d>%s | ", GREEN_BOLD_COLOR, spu->bytecode[i], COLOR_END); 
        else
            printf ("%s %04d %s | ", YELLOW_COLOR, spu->bytecode[i], COLOR_END); 
    }

    // printf("\n\tram[%lu]   \t = [%p]\n", SPU_MAX_RAM_SIZE, spu->ram);
    // printf("%s", "\t{");
    // for (size_t i = 0; i < SPU_VIDEO_HEIGHT; i++)
    // {
    //     printf ("\n\t\t[%04lu]: ", i);

    //     for (size_t j = 0; j < SPU_VIDEO_WIDTH; j++)
    //     {            
    //         printf ("%s %04d %s|", BLUE_COLOR, spu->ram[i], COLOR_END); 
    //     }
        
    // }
    // printf("%s", "\n\t}\n");

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
