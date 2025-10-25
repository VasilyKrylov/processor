#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "spu.h"

#include "spu_commands.h"

int DoPush (spu_t *spu)
{
    DEBUG_LOG ("%s", "PUSH");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for PUSH command");

        return RE_MISSING_ARGUMENT;
    }

    DEBUG_LOG ("\tPushed %d on stack", spu->bytecode[spu->ip]);

    int status = StackPush (&spu->stack, spu->bytecode[spu->ip]);
    if (status != STACK_OK)
    {
        StackPrintError (status);

        return status;
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

#define DO_MATH_OPERATION(functionName, mathOperation)                              \
int Do##functionName (spu_t *spu)                                                   \
{                                                                                   \
    DEBUG_LOG ("%s", "Do"#functionName);                                            \
                                                                                    \
    stackDataType a = 0;                                                            \
    stackDataType b = 0;                                                            \
                                                                                    \
    int status  = StackPop (&spu->stack, &a);                                       \
        status |= StackPop (&spu->stack, &b);                                       \
    if (status != STACK_OK)                                                         \
    {                                                                               \
        ERROR_PRINT ("%s", "ERROR in Do" #functionName " command");                 \
        ERROR_PRINT ("%s", "There are less than 2 elements on the stack");          \
                                                                                    \
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;                                     \
    }                                                                               \
    DEBUG_LOG ("\tstack->size = %lu", spu->stack.size);                             \
                                                                                    \
    DEBUG_LOG ("\ta, b: %d %d", a, b);                                              \
                                                                                    \
    StackPush (&spu->stack, b mathOperation a);                                     \
                                                                                    \
    spu->ip += 1;                                                                   \
                                                                                    \
    return RE_OK;                                                                   \
}
DO_MATH_OPERATION(Add, +)
DO_MATH_OPERATION(Sub, -)
DO_MATH_OPERATION(Mul, *)
#undef DO_MATH_OPERATION

int DoDiv (spu_t *spu)
{
    DEBUG_LOG ("%s", "DIV");

    stackDataType a = 0;
    stackDataType b = 0;

    int status  = StackPop (&spu->stack, &a);
        status |= StackPop (&spu->stack, &b);
    if (status != STACK_OK)
    {
        ERROR_PRINT ("%s", "ERROR in DIV command");
        ERROR_PRINT ("%s", "There are less than 2 elements on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }   

    if (a == 0)
    {
        ERROR_PRINT ("%s", "ERROR in DIV command");
        ERROR_PRINT ("%s", "Can't divide by zero");

        return RE_DIVISION_BY_ZERO;
    }

    StackPush (&spu->stack, b / a);
    
    DEBUG_LOG ("\ta, b: %d %d", a, b);
    
    spu->ip += 1;

    return RE_OK;
}

// no float
int DoSqrt (spu_t *spu)
{
    DEBUG_LOG ("%s", "SQRT");

    stackDataType a = 0;

    int status  = StackPop (&spu->stack, &a);
    if (status != STACK_OK)
    {
        ERROR_PRINT ("%s", "ERROR in SQRT command");
        ERROR_PRINT ("%s", "There is less than 1 element on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

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
    stackDataType outValue = 0;

    int status  = StackPop (&spu->stack, &outValue);
    if (status != STACK_OK)
    {
        ERROR_PRINT ("%s", "ERROR in OUT command");
        ERROR_PRINT ("%s", "There is less than 1 element on the stack");
        
        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

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
        ERROR_PRINT ("%s", "STUPID PIECE     OF SHIT, THIS IS NOT INTEGER NUMBER!");
        
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
        ERROR_PRINT ("%s", "Missing required argument for PUSHR command");

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    
    int regVal = spu->regs[regIdx]; // FIXME check

    DEBUG_LOG ("R%cX = %d;\n", char('A' + regIdx), regVal); // TODO: make function to get register name by its index

    DEBUG_LOG ("\tPushed %d on stack", regVal);
    int status = StackPush (&spu->stack, regVal);
    if (status != STACK_OK) 
    {        
        StackPrintError (status);

        return status;
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
        ERROR_PRINT ("%s", "Missing required argument for POPR command");

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    stackDataType stackValue = 0;

    int status = StackPop (&spu->stack, &stackValue);
    if (status != STACK_OK)
    {
        StackPrintError (status);

        return status;
    }

    spu->regs[regIdx] = stackValue;

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
    if ((size_t) spu->bytecode[spu->ip] >= spu->bytecodeCnt)
    {
        ERROR_PRINT ("JMP argument is grater than bytecode size (%d >= %lu)", 
                     spu->bytecode[spu->ip], spu->bytecodeCnt);
        
        return RE_JMP_ARGUMENT_IS_NEGATIVE;
    }
    // FIXME: spu->bytecode[spu->ip] больше размера байткода!!!!!!!!!!!!!!

    spu->ip = (size_t) spu->bytecode[spu->ip];
    
    return RE_OK;
}

#define DO_CONDITION_JMP(functionName, CMP)                         \
int Do##functionName (spu_t *spu)                                   \
{                                                                   \
    stackDataType first  = 0;                                       \
    stackDataType second = 0;                                       \
    int status = GetOperands (spu, &first, &second);                \
                                                                    \
    if (status != RE_OK)                                            \
        return status;                                              \
                                                                    \
    spu->ip++;                                                      \
                                                                    \
    if (second CMP first)                                           \
        spu->ip = (size_t) spu->bytecode[spu->ip];                  \
    else                                                            \
        spu->ip++;                                                  \
                                                                    \
    return RE_OK;                                                   \
}

DO_CONDITION_JMP (Jb,  CMP_BELOW)
DO_CONDITION_JMP (Jbe, CMP_BELOW_OR_EQUAL)
DO_CONDITION_JMP (Ja,  CMP_ABOVE)
DO_CONDITION_JMP (Jae, CMP_ABOVE_OR_EQUAL)
DO_CONDITION_JMP (Je,  CMP_EQUAL)
DO_CONDITION_JMP (Jne, CMP_NOT_EQUAL)

#undef DO_CONDITION_JMP

int GetOperands (spu_t *spu, stackDataType *first, stackDataType *second)
{
    int status = StackPop (&spu->stack, first);
    status    |= StackPop (&spu->stack, second);

    if (status & STACK_TRYING_TO_POP_FROM_EMPTY_STACK)
    {
        ERROR_PRINT ("%s", "There is not enought elements on stack for one of JUMP commands");

        return RE_NOT_ENOGUH_ELEMENTS_ON_STACK;
    }

    return RE_OK;
}

int DoCall (spu_t *spu)
{
    spu->ip++;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for CALL command");
        
        return RE_MISSING_ARGUMENT;
    }

    if (spu->bytecode[spu->ip] < 0)
    {
        ERROR_PRINT ("CALL argument is less than zero - %d", spu->bytecode[spu->ip]);
        
        return RE_CALL_ARGUMENT_IS_NEGATIVE;
    }
    if ((size_t) spu->bytecode[spu->ip] >= spu->bytecodeCnt)
    {
        ERROR_PRINT ("CALL argument is grater than bytecode size (%d >= %lu)", 
                     spu->bytecode[spu->ip], spu->bytecodeCnt);
        
        return RE_JMP_ARGUMENT_IS_NEGATIVE;
    }

    int status = StackPush (&spu->stackReturn, (int)spu->ip + 1);
    if (status != STACK_OK)
    {
        StackPrintError (status);

        return status;
    } // NOTE: maybe make macro for this ?
    
    spu->ip = (size_t) spu->bytecode[spu->ip];
    
    return RE_OK;
}

int DoRet (spu_t *spu)
{
    int returnAdress = -1;

    int status = StackPop (&spu->stackReturn, &returnAdress);
    if (status != STACK_OK)
    {
        StackPrintError (status);

        return status;
    }

    if (returnAdress < 0)
    {
        ERROR_PRINT ("RET: return adress is less than zero - %d", returnAdress);
        
        return RE_RET_VALUE_IS_NEGATIVE;
    }

    spu->ip = (size_t)returnAdress;

    return RE_OK;
}

int DoPushm (spu_t *spu)
{
    DEBUG_LOG ("%s", "PUSHM");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for PUSHM command");

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    int regVal = spu->regs[regIdx];
    int ramVal = spu->ram[regVal];

    DEBUG_LOG ("\tRAM[%d] = %d;\n", regVal, ramVal);
    DEBUG_LOG ("\tPushed %d on stack", ramVal);
    
    int status = StackPush (&spu->stack, ramVal);
    if (status != STACK_OK) 
    {
        StackPrintError (status);

        return status;
    }

    spu->ip += 1;

    return RE_OK;
}

int DoPopm (spu_t *spu)
{
    DEBUG_LOG ("%s", "POPM");

    spu->ip += 1;

    if (spu->ip >= spu->bytecodeCnt) 
    {
        ERROR_PRINT ("%s", "Missing required argument for POPM command");

        return RE_MISSING_ARGUMENT;
    }

    int regIdx = spu->bytecode[spu->ip];
    int regVal = spu->regs[regIdx];
    
    stackDataType stackValue = 0;

    int status = StackPop (&spu->stack, &stackValue);
    if (status != STACK_OK)
    {
        StackPrintError (status);

        return status;
    }

    spu->ram[regVal] = stackValue;

    DEBUG_LOG ("\tRAM[%d] = %d;\n", regVal, stackValue);

    spu->ip += 1;

    return RE_OK;
}

int DoDraw (spu_t *spu)
{
    for (size_t i = 0; i < SPU_MAX_RAM_SIZE; i++)
    {
        if (i % SPU_VIDEO_WIDTH == 0) 
            printf ("\n");
    
        printf ("%c ", spu->ram[i]);
    }

    printf ("\n");

    spu->ip++;

    usleep (100 * 1000);

    return SPU_OK;
}