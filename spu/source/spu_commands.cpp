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
        ERROR_PRINT ("%s", "Missing required argument for PUSH command")

        return RE_MISSING_ARGUMENT;
    }


    DEBUG_LOG ("\tPushed %d on stack", spu->bytecode[spu->ip]);
    int status = StackPush (&spu->stack, spu->bytecode[spu->ip]);
    if (status != STACK_OK)
    {
        DEBUG_LOG ("\tstatus = %d", status);
    }

    spu->ip += 1;

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

    spu->ip += 1;

    return status;
}

int DoAdd (spu_t *spu)
{
    DEBUG_LOG ("%s", "ADD");
    if (spu->stack.size < 2) // not here
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in ADD command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }
    DEBUG_LOG ("\tstack->size = %lu", spu->stack.size);

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a); // TODO: check here; what?
    StackPop (&spu->stack, &b);

    DEBUG_LOG ("\ta, b: %d %d", a, b);

    StackPush (&spu->stack, a + b);

    spu->ip += 1;

    return RE_OK;
}

int DoSub (spu_t *spu)
{
    DEBUG_LOG ("%s", "SUB");
    if (spu->stack.size < 2)
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in SUB command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a);
    StackPop (&spu->stack, &b);
    DEBUG_LOG ("\ta, b: %d %d", a, b);

    StackPush (&spu->stack, b - a);

    spu->ip += 1;

    return RE_OK;
}

int DoDiv (spu_t *spu)
{
    DEBUG_LOG ("%s", "DIV");
    if (spu->stack.size < 2)
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in DIV command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a);
    StackPop (&spu->stack, &b);


    if (a == 0)
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in DIV command, there are less than 2 elements on the stack");

        return RE_DIVISION_BY_ZERO;
    }

    StackPush (&spu->stack, b / a);
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);
    
    spu->ip += 1;

    return RE_OK;
}

int DoMul (spu_t *spu)
{
    DEBUG_LOG ("%s", "MUL");
    if (spu->stack.size < 2)
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in MUL command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

    stackDataType a = 0;
    stackDataType b = 0;

    StackPop (&spu->stack, &a); // TODO: check here
    StackPop (&spu->stack, &b);

    StackPush (&spu->stack, a * b);
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);

    spu->ip += 1;

    return RE_OK;
}

// no float
int DoSqrt (spu_t *spu)
{
    DEBUG_LOG ("%s", "SQRT");

    if (spu->stack.size < 1)
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in MUL command, there are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }
    stackDataType a = 0;

    StackPop (&spu->stack, &a);

    if (a < 0)
    {
        ERROR_PRINT ("%s", "Negative argument for SQRT command");

        return RE_SQRT_NEGATIVE_ARGUMENT;
    }

    StackPush (&spu->stack, (stackDataType) sqrt (a));

    DEBUG_LOG ("\tValue to calculate sqrt: %d", a);
    DEBUG_LOG ("\tSqrt: %d", (stackDataType) sqrt (a));
    
    spu->ip += 1;

    return RE_OK;
}

int DoOut (spu_t *spu)
{
    if (spu->bytecodeCnt < 1)
    {
        ERROR_PRINT ("%s", "ERROR_PRINT in OUT command, there is less than 1 element on the stack");

        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

    stackDataType outValue = 0;
    StackPop (&spu->stack, &outValue);

    printf ("OUT: %d\n", outValue); 

    spu->ip += 1;

    return RE_OK;
}

int DoIn (spu_t *spu)
{
    stackDataType inputValue = 0;

    printf ("%s", "Input integer number: ");
    int status = scanf (STACK_FORMAT_STRING, &inputValue);

    if (status != 1) 
    {
        ERROR_PRINT ("%s", "STUPID PEACE OF SHIT, THIS IS NOT INTEGER NUMBER!")
        
        return RE_INVALID_INPUT;
    }

    StackPush (&spu->stack, inputValue);

    DEBUG_PRINT ("inputValue = %d;\n", inputValue); 

    spu->ip += 1;

    return RE_OK;
}

int DoPushr (spu_t *spu)
{
    DEBUG_LOG ("%s", "PUSHR");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for PUSHR command")

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    int regVal = spu->regs[regIdx]; // FIXME check

    DEBUG_LOG ("R%cX = %d;\n", char('A' + regIdx), regVal); // TODO: make function to get register name by its index

    DEBUG_LOG ("\tPushed %d on stack", regVal);
    int status = StackPush (&spu->stack, regVal);
    if (status != STACK_OK) 
    {
        DEBUG_LOG ("\tstatus = %d", status);
    }

    spu->ip += 1;

    return RE_OK;
}

int DoPopr (spu_t *spu)
{
    DEBUG_LOG ("%s", "POPR");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for POPR command")

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    stackDataType stackValue = 0;

    int status = StackPop (&spu->stack, &stackValue);
    spu->regs[regIdx] = stackValue;

    if (status != STACK_OK)
    {
        DEBUG_LOG ("\tstack status = %d; // looks like not good program loaded to spu...", status);
    }

    DEBUG_LOG ("R%cX = %d;\n", char('A' + regIdx), spu->regs[regIdx]); // TODO: make function to get register name by its index

    spu->ip += 1;

    return RE_OK;
}

int DoJmp (spu_t *spu)
{
    spu->ip++;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for JMP command");
        
        return RE_MISSING_ARGUMENT;
    }

    if (spu->bytecode[spu->ip] < 0)
    {
        ERROR_PRINT ("JMP argument is less than zero - %d", spu->bytecode[spu->ip]);
        
        return RE_JMP_ARGUMENT_IS_NEGATIVE;
    }
    
    spu->ip = (size_t) spu->bytecode[spu->ip];
    
    return RE_OK;
}

int DoJb (spu_t *spu)
{
    spu->ip++;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for JUMP command")
        
        return RE_MISSING_ARGUMENT;
    }

    stackDataType first = 0;
    stackDataType second = 0;
    GetOperands (spu, &first, &second);

    if (second < first)
        spu->ip = (size_t) spu->bytecode[spu->ip];
    else
        spu->ip++;
    
    return RE_OK;
}

int GetOperands (spu_t *spu, stackDataType *first, stackDataType *second)
{
    int status = StackPop (&spu->stack, first);
    status    |= StackPop (&spu->stack, second);

    if (status & STACK_TRYING_TO_POP_FROM_EMPTY_STACK)
    {
        ERROR_PRINT ("%s", "There is not enought elements on stack for one of JUMP commands") // replace JUMP with JBE JB...

        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

    return RE_OK;
}