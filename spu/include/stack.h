#ifndef K_STACK_H
#define K_STACK_H

#include <stdio.h>

#include "debug.h"

typedef int stackDataType;
const char * const stackDataTypeStr = "int";
#define STACK_FORMAT_STRING "%d"
const stackDataType CANARY = (stackDataType) 0xEDAc0ffe;
const stackDataType POISON = 1337228272;

#ifdef PRINT_DEBUG
    struct varInfo_t // FIXME: move to common
    {
        const char *name;
        const char *file;
        int line;
        const char *func;
    };

    #define STACK_DUMP(stackName, comment) StackDump (&stackName, comment, __FILE__, __LINE__, __func__);

    #define STACK_CREATE(stackName, size) StackCtor (stackName, size,                         \
                                                        varInfo_t{.name = #stackName,         \
                                                                .file = __FILE__,             \
                                                                .line = __LINE__,             \
                                                                .func = __func__});
    #define STACK_ERROR(stack) StackError (stack);  
#else
    #define STACK_DUMP(stack, comment) 
    #define STACK_CREATE(stackName, size) StackCtor (stackName, size);
    #define STACK_ERROR(stack) OK;
#endif // PRINT_DEBUG

struct stack_t 
{
#ifdef STACK_CANARY
    stackDataType canaryStart = (stackDataType) CANARY;
#endif // STACK_CANARY

    stackDataType *data = NULL;
    size_t size = 0;
    size_t capacity = 0;

#ifdef PRINT_DEBUG
    varInfo_t varInfo;
#endif // PRINT_DEBUG

#ifdef STACK_CANARY
    stackDataType canaryEnd = (stackDataType) CANARY;
#endif // STACK_CANARY
};

// TODO: add class to other enum's

enum stackErrors // FIXME: make enum class
{
    OK                              = 0,
    NULL_STRUCT                     = 1 << 0,
    NULL_DATA                       = 1 << 1,
    STACK_OVERFLOW                  = 1 << 2,
    BIG_CAPACITY                    = 1 << 3,
    STRUCT_CANARY_START_OVERWRITE   = 1 << 4,
    STRUCT_CANARY_END_OVERWRITE     = 1 << 5,
    DATA_CANARY_START_OVERWRITE     = 1 << 6,
    DATA_CANARY_END_OVERWRITE       = 1 << 7,
    POISON_VALUE_IN_DATA            = 1 << 8,
    WRONG_VALUE_IN_POISON           = 1 << 9,
    TRYING_TO_POP_FROM_EMPTY_STACK  = 1 << 10 
};
void StackPrintError (int error);
int StackError (stack_t *stack);
int StackCtor (stack_t *stack, size_t capacity
                  ON_DEBUG(, varInfo_t varInfo));
int StackPush (stack_t *stack, stackDataType value);
int StackPop (stack_t *stack, stackDataType *value);
int StackDtor (stack_t *stack);
void StackDump (stack_t *stack, const char *comment,
                const char *file, int line, const char * func);

#endif // K_STACK_H