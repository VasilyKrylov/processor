#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stack.h"

const size_t maxCapacity = (1UL << 32);

#ifdef STACK_CANARY
stackDataType *GetCanaryStart (stack_t *stack);
stackDataType *GetCanaryEnd (stack_t *stack);

stackDataType *GetCanaryStart (stack_t *stack)
{
    return stack->data - 1;
}
stackDataType *GetCanaryEnd (stack_t *stack)
{
    return stack->data + stack->capacity;
}
#endif // STACK_CANARY

stackDataType *GetOrigData (stackDataType *data);
size_t GetCapacity (stack_t *stack);

stackDataType *GetOrigData (stackDataType *data)
{
#ifdef STACK_CANARY
    return data - 1;
#else
    return data;
#endif // STACK_CANARY
}
size_t GetCapacity (stack_t *stack)
{
#ifdef STACK_CANARY
    return stack->capacity + 2;
#else
    return stack->capacity;
#endif
}

void StackPrintError (int error)
{
    DEBUG_LOG ("error = %d\n", error);

    if (error == 0) return;

    printf("%s", RED_BOLD_COLOR);

    if (error & STACK_NULL_STRUCT)                    printf("%s", "Stack struct is null!\n");
    if (error & STACK_NULL_DATA)                      printf("%s", "Stack data is null!\n");
    if (error & STACK_OVERFLOW)                         printf("%s", "Stack size grater than stack capacity\n");
    if (error & STACK_BIG_CAPACITY)	                    printf("%s", "Stack capacity is too big, sorry bro\n");
    if (error & STACK_STRUCT_CANARY_START_OVERWRITE)	printf("%s", "Beginning of the stack structure has been overwritten\n");
    if (error & STACK_STRUCT_CANARY_END_OVERWRITE)	    printf("%s", "End of the stack structure has been overwritten\n");
    if (error & STACK_DATA_CANARY_START_OVERWRITE)	    printf("%s", "Beginning of the data field has been overwritten\n");
    if (error & STACK_DATA_CANARY_END_OVERWRITE)	    printf("%s", "End of the data field has been overwritten\n");
    if (error & STACK_POISON_VALUE_IN_DATA)	            printf("%s", "There is poison value in stack\n");
    if (error & STACK_WRONG_VALUE_IN_POISON)	        printf("%s", "There is NOT poison value in unused section of stack\n");
    if (error & STACK_TRYING_TO_POP_FROM_EMPTY_STACK)	printf("%s", "Stack is empty, but StackPop() called\n");

    printf("%s", COLOR_END);
}

int StackError (stack_t *stack)
{
    int error = STACK_OK;

    if (stack == NULL)
    {
        error |= STACK_NULL_STRUCT;
        return error;
    }
    if (stack->size > stack->capacity)
    {
        error |= STACK_OVERFLOW;
    }
    if (stack->capacity > maxCapacity)
    {
        error |= STACK_BIG_CAPACITY;
    }
    if (stack->data == NULL)
    {
        error |= STACK_NULL_DATA;
        return error;
    }

#ifdef STACK_CANARY
    if (stack->canaryStart != CANARY) 
    {
        error |= STACK_STRUCT_CANARY_START_OVERWRITE;
    }
    if (stack->canaryEnd != CANARY)
    {
        error |= STACK_STRUCT_CANARY_END_OVERWRITE;
    }
    if (*GetCanaryStart (stack) != CANARY)
    {
        error |= STACK_DATA_CANARY_START_OVERWRITE;
    }
    if (*GetCanaryEnd (stack) != CANARY)
    {
        error |= STACK_DATA_CANARY_END_OVERWRITE;
    }
#endif // STACK_CANARY

#ifdef PRINT_DEBUG
    for (size_t i = 0; i < stack->size; i++)
    {
        if (stack->data[i] == POISON)
        {
            error |= STACK_POISON_VALUE_IN_DATA;
            break;
        }
    }
    for (size_t i = stack->size; i < stack->capacity; i++)
    {
        if (stack->data[i] != POISON)
        {
            error |= STACK_WRONG_VALUE_IN_POISON;
            break;
        }
    }
#endif //PRINT_DEBUG

    return error;
}

int StackCtor (stack_t *stack, size_t capacity
               ON_DEBUG (, varInfo_t varInfo))
{
    if (stack == NULL)
        return STACK_NULL_STRUCT;
    if (capacity > maxCapacity)
        return STACK_BIG_CAPACITY;
    
    stack->size = 0;
    stack->capacity = capacity;
    stack->data = (stackDataType *) calloc (GetCapacity (stack), sizeof(stackDataType));

#ifdef STACK_CANARY
    stack->data += 1;
#endif // STACK_CANARY
    
    if (stack->data == NULL)
        return STACK_NULL_DATA;


#ifdef STACK_CANARY
    DEBUG_LOG ("canary end %p\n", GetCanaryEnd (stack));
    DEBUG_LOG ("canary end idx in bytes %ld\n", GetCanaryEnd (stack) - (int *)stack);
    // DEBUG_LOG ("canary end value %d\n", *GetCanaryEnd (stack));
    
    *GetCanaryStart (stack) = CANARY;
    *GetCanaryEnd (stack)   = CANARY;
#endif // STACK_CANARY

#ifdef PRINT_DEBUG
    stack->varInfo = varInfo;
    for (size_t i = 0; i < capacity; i++)
    {
        stack->data[i] = POISON;
    }
#endif // PRINT_DEBUG

    return STACK_ERROR (stack);
}

int StackPush (stack_t *stack, stackDataType value)
{
    int error = STACK_ERROR (stack);
    if (error != STACK_OK) 
        return error;

    if (stack->size == stack->capacity)
    {
        stack->capacity *= 2;
        if (stack->capacity == 0)
            stack->capacity = 2;

        stackDataType *newData = (stackDataType *) realloc (GetOrigData (stack->data), 
                                                            GetCapacity (stack) * sizeof(stackDataType));
#ifdef STACK_CANARY
        newData += 1;
#endif // STACK_CANARY

        if (GetOrigData(newData) == NULL)
            return STACK_NULL_DATA;

        stack->data = newData;
#ifdef STACK_CANARY
    *GetCanaryEnd (stack) = CANARY;
#endif // STACK_CANARY
    }

#ifdef PRINT_DEBUG
    for (size_t i = stack->size + 1; i < stack->capacity; i++)
    {
        stack->data[i] = POISON;
    }
#endif // PRINT_DEBUG

    stack->data[stack->size] = value;
    stack->size++;

    return STACK_ERROR (stack);
}

int StackPop (stack_t *stack, stackDataType *value)
{
    int error = STACK_ERROR (stack);
    if (error != STACK_OK) 
        return error;

    if (stack->size == 0)
        return STACK_TRYING_TO_POP_FROM_EMPTY_STACK;

    if (stack->size * 4 <= stack->capacity)
    {
        stack->capacity /= 2;

        stackDataType *newData = (stackDataType *) realloc (GetOrigData (stack->data), 
                                                            GetCapacity(stack) * sizeof(stackDataType));
#ifdef STACK_CANARY
        newData += 1;
#endif // STACK_CANARY

        if (GetOrigData (newData) == NULL)
            return STACK_NULL_DATA;

        stack->data = newData;
        
#ifdef STACK_CANARY
    *GetCanaryEnd (stack) = CANARY;
#endif // STACK_CANARY
    }

    stack->size--;
    *value = stack->data[stack->size];
#ifdef PRINT_DEBUG
    stack->data[stack->size] = POISON;
#endif // PRINT_DEBUG
    return STACK_ERROR (stack);
}

int StackDtor (stack_t *stack)
{
    if (stack == NULL)
        return STACK_NULL_STRUCT;

    int error = STACK_ERROR (stack);

    stack->size     = 0;
    stack->capacity = 0;
    
    stackDataType *origData = GetOrigData (stack->data);
    free (origData);
    stack->data = NULL;

    return error;
}

// make output to file
void StackDump (stack_t *stack, const char *comment,
                const char *file, int line, const char * func)
{
    if (stack == NULL)
    {
        ERROR("%s", "stack is NULL")
        return;
    }
    
    printf("stack<%s> [%p]", stackDataTypeStr, stack);
#ifdef PRINT_DEBUG
    printf(" \"%s\" %s:%d %s()", stack->varInfo.name,
                                 stack->varInfo.file,
                                 stack->varInfo.line,
                                 stack->varInfo.func);
#endif // PRINT_DEBUG
    puts("");
    printf("called at %s:%d %s()\n", file, line, func);
    printf("reason: \"%s\"\n", comment);

    printf("%s", "{\n");
    printf("\tcapacity \t= %lu\n", stack->capacity);
    printf("\tsize     \t= %lu\n", stack->size);
    printf("\tdata[%lu]\t= [%p]\n", stack->capacity, stack->data);

    printf("%s", "\t{\n");

#ifdef STACK_CANARY
    printf ("\t\tcanaryStart = 0x%x\n", 
            (unsigned int) *GetOrigData(stack->data));
#endif
    for (size_t i = 0; i < stack->capacity; i++)
    {
        if (i < stack->size)
            printf("%s", "\t\t*");
        else
            printf("%s", "\t\t ");

        printf("[%lu] = " STACK_FORMAT_STRING "", i, stack->data[i]);

        if (i >= stack->size)
            printf("%s", " (POSION)");
        
        printf("%s", "\n");
    }
#ifdef STACK_CANARY
    printf ("\t\tcanaryEnd = 0x%x\n", 
            (unsigned int) stack->data[stack->capacity]);
#endif

    printf("%s", "\t}\n");

    printf("%s", "}\n");
}