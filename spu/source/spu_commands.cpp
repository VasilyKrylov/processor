#include <stdio.h>
#include <math.h>

#include "spu_commands.h"

#include "spu.h"

int DoPush (spu_t *processor)
{
    DEBUG_LOG ("%s", "PUSH");

    if (processor->ip + 1 >= processor->bytecodeCnt) 
    {
        ERROR ("%s", "Missing required argument for PUSH command")

        return RE_MISSING_ARGUMENT;
    }

    processor->ip += 1;

    DEBUG_LOG ("\tPushed %d on stack", processor->bytecode[processor->ip]);
    int status = StackPush (&processor->stack, processor->bytecode[processor->ip]);
    if (status != RE_OK) 
    {
        DEBUG_LOG ("\tstatus = %d", status);
    }

    return RE_OK;
}

int DoPop (spu_t *processor)
{
    DEBUG_LOG ("%s", "POP");
    stackDataType a = 0;
    int status = StackPop (&processor->stack,  &a);

    if (status != RE_OK)
    {
        DEBUG_LOG ("\tstack status = %d; // looks like not good program loaded to spu...", status);
    }

    return status;
}

int DoAdd (spu_t *processor)
{
    DEBUG_LOG ("%s", "ADD");
    if (processor->stack.size < 2) // not here
    {
        ERROR ("%s", "Error in ADD command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }
    DEBUG_LOG ("\tstack->size = %lu", processor->stack.size);

    stackDataType a = 0;
    stackDataType b = 0;


    StackPop (&processor->stack, &a); // TODO: check here; what?
    StackPop (&processor->stack, &b);

    DEBUG_LOG ("\ta, b: %d %d", a, b);

    StackPush (&processor->stack, a + b);

    return RE_OK;
}

int DoSub (spu_t *processor)
{
    DEBUG_LOG ("%s", "SUB");
    if (processor->stack.size < 2)
    {
        ERROR ("%s", "Error in SUB command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&processor->stack, &a);
    StackPop (&processor->stack, &b);
    DEBUG_LOG ("\ta, b: %d %d", a, b);

    StackPush (&processor->stack, b - a);
    return RE_OK;
}

int DoDiv (spu_t *processor)
{
    DEBUG_LOG ("%s", "DIV");
    if (processor->stack.size < 2)
    {
        ERROR ("%s", "Error in DIV command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&processor->stack, &a);
    StackPop (&processor->stack, &b);


    if (a == 0)
    {
        ERROR ("%s", "Error in DIV command, there are less than 2 elements on the stack");
    }
    StackPush (&processor->stack, b / a); // TODO: add check a != 0
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);
    
    return RE_OK;
}

int DoMul (spu_t *processor)
{
    DEBUG_LOG ("%s", "MUL");
    if (processor->stack.size < 2)
    {
        ERROR ("%s", "Error in MUL command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&processor->stack, &a); // TODO: should I check status?
    StackPop (&processor->stack, &b);

    StackPush (&processor->stack, a * b);
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);

    return RE_OK;
}

// no float
int DoSqrt (spu_t *processor)
{
    DEBUG_LOG ("%s", "SQRT");

    if (processor->stack.size < 1)
    {
        ERROR ("%s", "Error in MUL command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }
    stackDataType a = 0;

    StackPop (&processor->stack, &a);

    if (a < 0)
    {
        ERROR ("%s", "Negative argument for SQRT command");

        return RE_SQRT_NEGATIVE_ARGUMENT;
    }

    StackPush (&processor->stack, (stackDataType) sqrt (a));

    DEBUG_LOG ("\tValue to calculate sqrt: %d", a);
    DEBUG_LOG ("\tSqrt: %d", (stackDataType) sqrt (a));
    
    return RE_OK;
}

int DoOut (spu_t *processor)
{
    if (processor->bytecodeCnt < 1)
    {
        ERROR ("%s", "Error in OUT command, there is less than 1 element on the stack");

        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType outValue = 0;
    StackPop (&processor->stack, &outValue);

    printf ("%d\n", outValue); // TODO: check printf for error?

    return RE_OK;
}

int DoIn (spu_t *processor)
{
    stackDataType inputValue = 0;

    printf ("%s", "Input integer number: ");
    int status = scanf ("" STACK_FORMAT_STRING "", &inputValue);

    if (status != 1) 
    {
        ERROR ("%s", "STUPID PEACE OF SHIT, THIS IS NOT INTEGER NUMBER!")
        
        return RE_INVALID_INPUT;
    }

    StackPush (&processor->stack, inputValue);

    printf ("inputValue = %d;\n", inputValue); // TODO: check printf for error?

    return RE_OK;
}