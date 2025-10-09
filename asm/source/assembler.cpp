#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

#include "spu_common.h"
#include "file.h"
#include "debug.h"
#include "utils.h"

struct command_t 
{
    const char *name = "";
    int bytecode = 0;
    int requireArgument = 0;
};

int AssembleCommand (asm_t *myAsm, const command_t *command, 
                     char *lineStart, size_t lineNumber);
bool IsRegisterCommand (int command);
int GetRegisterBytecode (char *registerName, int *bytecode);

// TODO: change requireArgument to int
static const command_t commands[] = {
    {.name = "PUSH",  .bytecode =  SPU_PUSH,    .requireArgument = 1}, 
    {.name = "POP",   .bytecode =  SPU_POP,     .requireArgument = 0}, // NOTE OPTIONAL: make requireArgument int
    {.name = "ADD",   .bytecode =  SPU_ADD,     .requireArgument = 0}, 
    {.name = "SUB",   .bytecode =  SPU_SUB,     .requireArgument = 0},
    {.name = "DIV",   .bytecode =  SPU_DIV,     .requireArgument = 0},
    {.name = "MUL",   .bytecode =  SPU_MUL,     .requireArgument = 0},
    {.name = "SQRT",  .bytecode =  SUP_SQRT,    .requireArgument = 0},
    {.name = "OUT",   .bytecode =  SPU_OUT,     .requireArgument = 0},
    {.name = "IN",    .bytecode =  SPU_IN,      .requireArgument = 0},
    {.name = "PUSHR", .bytecode =  SPU_PUSHR,   .requireArgument = 1},
    {.name = "POPR",  .bytecode =  SPU_POPR,    .requireArgument = 1},
    {.name = "JMP",   .bytecode =  SPU_JMP,     .requireArgument = 1},
    {.name = "JB",    .bytecode =  SPU_JB,      .requireArgument = 1},
    {.name = "JBE",   .bytecode =  SPU_JBE,     .requireArgument = 1},
    {.name = "JA",    .bytecode =  SPU_JA,      .requireArgument = 1},
    {.name = "JAE",   .bytecode =  SPU_JAE,     .requireArgument = 1},
    {.name = "JE",    .bytecode =  SPU_JE,      .requireArgument = 1},
    {.name = "JNE",   .bytecode =  SPU_JNE,     .requireArgument = 1},
    {.name = "HLT",   .bytecode =  SPU_HLT,     .requireArgument = 0},
    // TODO: COMMAND (IN, 0)
};

static const size_t CommandsNumber = sizeof(commands) / sizeof(command_t);

int AsmRead (asm_t *myAsm, char *inputFileName)
{
    DEBUG_LOG ("%s", "AsmRead()");

    int status = TextCtor (inputFileName, &(myAsm->text));

    DEBUG_LOG ("buffer: \"%s\"\n", myAsm->text.buffer);
    
    if (status != 0)
    {
        return 1;
    }
    
    myAsm->fileName = inputFileName;
    // myAsm->instructionsCnt = CountOperands (myAsm->text.buffer, " \n"); 
    myAsm->instructionsCnt = 8; 
    myAsm->bytecode = (int *) calloc (myAsm->instructionsCnt, sizeof(int));

    DEBUG_LOG ("myAsm->instructionsCnt: %lu\n", myAsm->instructionsCnt);
    
    StrReplace (myAsm->text.buffer, "\n;", '\0');

    return 0;
}

// TODO: check for this errors:
// ADD 10
// PUSH 20 20
// PUSHR RAX RBX
// TODO: make functions:  AssembleArgument
//                        Resize
int AssembleCommand (asm_t *myAsm, const command_t *command, 
                     char *lineStart, size_t lineNumber)
{

    if (myAsm->ip + 1 >= myAsm->instructionsCnt)
    {
        myAsm->instructionsCnt *= 2;

        int *newBytecode = (int *) realloc (myAsm->bytecode, sizeof(int) * myAsm->instructionsCnt); // TODO: memset()?
        if (newBytecode == NULL)
        {
            perror ("Error trying to realloc myAsm->bytecode");

            return MY_ASM_COMMON_ERROR |
                   COMMON_ERROR_REALLOCATING_MEMORY;
        }

        myAsm->bytecode = newBytecode;
    }

    myAsm->bytecode[myAsm->ip] = command->bytecode;

    myAsm->ip++;

    int argumentBytecode = -666;
    
    if (command->requireArgument)
    {
        int readedBytes = 0;

        if (IsRegisterCommand (command->bytecode)) 
        {
            char regStr[8] = {}; // TODO: it should be 4, but i recive
            // warning: stack protector not protecting function: all local arrays are less than 8 bytes long [-Wstack-protector]
            
            int res = sscanf (lineStart, "%3s %n", regStr, &readedBytes); 
            lineStart += readedBytes;

            if (res != 1)
            {
                ERROR_PRINT ("%s:%lu Error reading register name, whish is required for %s command\n",
                             myAsm->fileName, lineNumber, command->name)
                
                return MY_ASM_INVALID_REGISTER_NAME;
            }

            int result = GetRegisterBytecode (regStr, &argumentBytecode);
            if (result != MY_ASM_OK)
            {
                ERROR_PRINT ("%s:%lu Invalid register name. Correct one for exammple is \"RAX\"\n",
                             myAsm->fileName, lineNumber)

                return result;
            }
        }
        else
        {
            int res = sscanf (lineStart, "%d %n", &argumentBytecode, &readedBytes);
            lineStart += readedBytes;

            if (res != 1)
            {
                ERROR_PRINT ("%s:%lu Error reading command argument, whish is required for %s command\n",
                             myAsm->fileName, lineNumber, command->name)

                return MY_ASM_MISSING_ARGUMENT;
            }
        }

        // FIXME: just go over linesStart checking for not probel
        char trash = '\0';
        int res = sscanf (lineStart, "%s %n", &trash, &readedBytes);
        lineStart += readedBytes;

        if (res == 1)
        {
            ERROR_PRINT ("%s:%lu Extra argument after command %s, \'%c\'=%d\n",
                            myAsm->fileName, lineNumber, command->name, trash, trash)

            return MY_ASM_EXTRA_ARGUMENT;
        }
        //

        DEBUG_PRINT ("argument = %d;\n", argumentBytecode);
        
        //delete this resize
        if (myAsm->ip >= myAsm->instructionsCnt)
        {
            myAsm->instructionsCnt *= 2;

            int *newBytecode = (int *) realloc (myAsm->bytecode, sizeof(int) * myAsm->instructionsCnt);
            if (newBytecode == NULL)
            {
                perror ("Error trying to realloc myAsm->bytecode");

                return MY_ASM_COMMON_ERROR; // TODO: but I want COMMON_ERROR_REALLOCATING_MEMORY
            }

            myAsm->bytecode = newBytecode;
        }

        myAsm->bytecode[myAsm->ip] = argumentBytecode;

        myAsm->ip++;
    }

    //check for trash symbols

    return MY_ASM_OK;
}

int Assemble (asm_t *myAsm)
{
    myAsm->ip = 0;

    for (size_t i = 0; myAsm->text.lines[i].start != 0; i++)
    {
        DEBUG_PRINT ("[%lu].len: %lu\n", i, myAsm->text.lines[i].len);
        DEBUG_PRINT ("[%lu].start: %p\n", i, myAsm->text.lines[i].start);
        DEBUG_PRINT ("[%lu] = \"%.*s\"\n", i, (int) myAsm->text.lines[i].len - 1, myAsm->text.lines[i].start);

        char *lineStart = myAsm->text.lines[i].start;

        if (lineStart[0] == '\0') 
            continue;

        bool knownCommand = false;
        int readedBytes = 0;
        char command[MAX_COMMAND_LEN] = {}; // FIXME: constant for command!!!!!!!!!!!

        // FIXME: just use strncmp bro
        sscanf (lineStart, "%" "s %n", command, &readedBytes);
        
        lineStart += readedBytes;

        DEBUG_PRINT ("command = \"%s\", readedBytes = %d;\n", command, readedBytes);
        
        // TODO: make this for-loop in another function
        for (size_t j = 0; j < CommandsNumber; j++)
        {
            if (strcmp (commands[j].name, command) == 0)
            {
                knownCommand = true;

                int res = AssembleCommand (myAsm, &commands[j], lineStart, i + 1);

                if (res != MY_ASM_OK) return res;
            }

        }

        if (!knownCommand) 
        {
            ERROR_PRINT ("%s:%lu Uknown command \"%s\"", myAsm->fileName, i + 1, command);

            return MY_ASM_UKNOWN_COMMAND;
        }

        DEBUG_PRINT ("%s", "\n");
    }

    return MY_ASM_OK;
}

int PrintBytecode (const char *outputFileName, asm_t *myAsm)
{
    FILE *outputFile = fopen (outputFileName, "w");
    if (outputFile == NULL)
    {
        perror ("Error opening output file");
        return -1;
    }

    for (size_t i = 0; i < myAsm->instructionsCnt; i++)
    {
        int status = fprintf (outputFile, "%d ", myAsm->bytecode[i]);
        
        if (status < 0)
        {
            fprintf (stderr, "Error while writing output to %s", outputFileName);
            DEBUG_LOG ("i = %lu;", i);
            
            return status;
        }

        // if (myAsm->bytecode[i] == SPU_HLT)
        //     break;
    }

    DEBUG_LOG ("%s", "PrintBytecode() returns 0");

    return 0;
}

bool IsRegisterCommand (int command)
{
    const int registerBit = 1 << 5;

    return (command & registerBit);
}

// RAX -> 0
// RBX -> 1 
// ..
// RHX -> 7
// all other cases will be errors
int GetRegisterBytecode (char *registerName, int *bytecode)
{
    if (registerName[0] != 'R') return MY_ASM_INVALID_REGISTER_NAME;
    if (registerName[2] != 'X') return MY_ASM_INVALID_REGISTER_NAME;

    int registerIdx = registerName[1] - 'A';

    if (registerIdx < 0)                  return MY_ASM_INVALID_REGISTER_NAME;
    if ((size_t)registerIdx >= NUMBER_OF_REGISTERS) return MY_ASM_INVALID_REGISTER_NAME;

    *bytecode = registerIdx;

    return MY_ASM_OK;
}

int AsmPrintError (int error)
{
    DEBUG_LOG ("Assembler error = %d\n", error);

    if (error == MY_ASM_OK) return 0;

    fprintf(stderr, "%s", RED_BOLD_COLOR);

    if (error & MY_ASM_UKNOWN_COMMAND)        fprintf(stderr, "%s", "Uknown command in input file\n");
    if (error & MY_ASM_MISSING_ARGUMENT)      fprintf(stderr, "%s", "Missing argument for command that requires it\n");
    if (error & MY_ASM_EXTRA_ARGUMENT)        fprintf(stderr, "%s", "Extra argument for command that doesn't requires it\n");
    if (error & MY_ASM_INVALID_REGISTER_NAME) fprintf(stderr, "%s", "Invalid register name. Correct one for exammple is \"RAX\"\n");
    
    fprintf(stderr, "%s", COLOR_END);
    
    return 0;
}

int AsmDtor (asm_t *myAsm)
{
    TextDtor (&(myAsm->text));

    if (myAsm->bytecode != NULL)
    {
        free (myAsm->bytecode);
        myAsm->bytecode = NULL;
    }

    myAsm->instructionsCnt = 0;

    myAsm->fileName = NULL;

    return 0;
}