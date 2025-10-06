#include <stdio.h>
#include <math.h>

#include "spu_commands.h"

#include "spu.h"

int DoPush (spu_t *spu)
{
    DEBUG_LOG ("%s", "PUSH");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR ("%s", "Missing required argument for PUSH command")

        return RE_MISSING_ARGUMENT;
    }


    DEBUG_LOG ("\tPushed %d on stack", spu->bytecode[spu->ip]);
    int status = StackPush (&spu->stack, spu->bytecode[spu->ip]);
    if (status != OK) // FIXME: OK = STACK_OK
    {
        DEBUG_LOG ("\tstatus = %d", status);
    }

    return RE_OK;
}

int DoPop (spu_t *spu)
{
    DEBUG_LOG ("%s", "POP");
    stackDataType a = 0;
    int status = StackPop (&spu->stack,  &a);

    if (status != RE_OK)
    {
        DEBUG_LOG ("\tstack status = %d; // looks like not good program loaded to spu...", status);
    }

    return status;
}

int DoAdd (spu_t *spu)
{
    DEBUG_LOG ("%s", "ADD");
    if (spu->stack.size < 2) // not here
    {
        ERROR ("%s", "Error in ADD command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }
    DEBUG_LOG ("\tstack->size = %lu", spu->stack.size);

    stackDataType a = 0;
    stackDataType b = 0;


    StackPop (&spu->stack, &a); // TODO: check here; what?
    StackPop (&spu->stack, &b);

    DEBUG_LOG ("\ta, b: %d %d", a, b);

    StackPush (&spu->stack, a + b);

    return RE_OK;
}

int DoSub (spu_t *spu)
{
    DEBUG_LOG ("%s", "SUB");
    if (spu->stack.size < 2)
    {
        ERROR ("%s", "Error in SUB command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a);
    StackPop (&spu->stack, &b);
    DEBUG_LOG ("\ta, b: %d %d", a, b);

    StackPush (&spu->stack, b - a);
    return RE_OK;
}

int DoDiv (spu_t *spu)
{
    DEBUG_LOG ("%s", "DIV");
    if (spu->stack.size < 2)
    {
        ERROR ("%s", "Error in DIV command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a);
    StackPop (&spu->stack, &b);


    if (a == 0)
    {
        ERROR ("%s", "Error in DIV command, there are less than 2 elements on the stack");
    }
    StackPush (&spu->stack, b / a); // TODO: add check a != 0
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);
    
    return RE_OK;
}

int DoMul (spu_t *spu)
{
    DEBUG_LOG ("%s", "MUL");
    if (spu->stack.size < 2)
    {
        ERROR ("%s", "Error in MUL command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a); // TODO: should I check status?
    StackPop (&spu->stack, &b);

    StackPush (&spu->stack, a * b);
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);

    return RE_OK;
}

// no float
int DoSqrt (spu_t *spu)
{
    DEBUG_LOG ("%s", "SQRT");

    if (spu->stack.size < 1)
    {
        ERROR ("%s", "Error in MUL command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }
    stackDataType a = 0;

    StackPop (&spu->stack, &a);

    if (a < 0)
    {
        ERROR ("%s", "Negative argument for SQRT command");

        return RE_SQRT_NEGATIVE_ARGUMENT;
    }

    StackPush (&spu->stack, (stackDataType) sqrt (a));

    DEBUG_LOG ("\tValue to calculate sqrt: %d", a);
    DEBUG_LOG ("\tSqrt: %d", (stackDataType) sqrt (a));
    
    return RE_OK;
}

int DoOut (spu_t *spu)
{
    if (spu->bytecodeCnt < 1)
    {
        ERROR ("%s", "Error in OUT command, there is less than 1 element on the stack");

        return RE_NOT_ENOGUH_ELEMENTS_IN_STACK;
    }

    stackDataType outValue = 0;
    StackPop (&spu->stack, &outValue);

    printf ("%d\n", outValue); // TODO: check printf for error?

    return RE_OK;
}

int DoIn (spu_t *spu)
{
    stackDataType inputValue = 0;

    printf ("%s", "Input integer number: ");
    int status = scanf ("" STACK_FORMAT_STRING "", &inputValue);

    if (status != 1) 
    {
        ERROR ("%s", "STUPID PEACE OF SHIT, THIS IS NOT INTEGER NUMBER!")
        
        return RE_INVALID_INPUT;
    }

    StackPush (&spu->stack, inputValue);

    DEBUG_PRINT ("inputValue = %d;\n", inputValue); // TODO: check printf for error?

    return RE_OK;
}

int DoPushr (spu_t *spu)
{
    DEBUG_LOG ("%s", "PUSHR");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR ("%s", "Missing required argument for PUSHR command")

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    int regVal = spu->regs[regIdx];

    DEBUG_LOG ("R%cX = %d;\n", char('A' + regIdx), regVal); // TODO: make function to get register name by its index

    DEBUG_LOG ("\tPushed %d on stack", regVal);
    int status = StackPush (&spu->stack, regVal);
    if (status != OK) 
    {
        DEBUG_LOG ("\tstatus = %d", status);
    }

    return RE_OK;
}

int DoPopr (spu_t *spu)
{
    DEBUG_LOG ("%s", "POPR");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR ("%s", "Missing required argument for POPR command")

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    stackDataType stackValue = 0;

    int status = StackPop (&spu->stack, &stackValue);
    spu->regs[regIdx] = stackValue;

    if (status != OK)
    {
        DEBUG_LOG ("\tstack status = %d; // looks like not good program loaded to spu...", status);
    }

    DEBUG_LOG ("R%cX = %d;\n", char('A' + regIdx), spu->regs[regIdx]); // TODO: make function to get register name by its index

    return RE_OK;
}