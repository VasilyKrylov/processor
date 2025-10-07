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

// TODO: change requireArgument to int
static const command_t commands[] = {
    {.name = "PUSH",  .bytecode =  SPU_PUSH,    .requireArgument = 1}, // TODO: enum for bytecodes in common
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
    {.name = "JUMP",  .bytecode =  SPU_JMP,    .requireArgument = 1},
    {.name = "HLT",   .bytecode =  SPU_HLT,     .requireArgument = 0},
};

static const size_t CommandsNumber = sizeof(commands) / sizeof(command_t);

size_t CountOperands (char *s, const char *delimiter);

int AsmCtorAndRead (char *inputFileName, asm_t *myAsm)
{
    DEBUG_LOG ("%s", "AsmCtorAndRead()");

    int status = TextCtor (inputFileName, &(myAsm->text));

    DEBUG_LOG ("buffer: \"%s\"\n", myAsm->text.buffer);
    
    if (status != 0)
    {
        return 1;
    }
    
    // TODO: replace \n with \0
    // TODO: replace ; with \0

    myAsm->instructionsCnt = CountOperands (myAsm->text.buffer, " \n"); // TODO: make constant

    DEBUG_LOG ("myAsm->instructionsCnt: %lu\n", myAsm->instructionsCnt);

    return 0;
}

// FIXME: split on functions  !!!!!!!!!!!!!!!!!
// FIXME: replace strok_r with sscanf() and onegin functions !!!!!!!!!!!!!!!!
int Assemble (asm_t *myAsm)
{
    DEBUG_LOG ("buffer: \"%s\"\n", myAsm->text.buffer);

    size_t bytecodeIdx = 0;
    myAsm->bytecode = (int *) calloc (myAsm->instructionsCnt, sizeof(int));

    const char *commandDelimiter = "\n";
    const char *argumentsDelimiter = " ";

    char *commandsSavePtr = NULL;

    // linesArray and sscanf
    size_t lineNumber = 1;
    char *line = strtok_r (myAsm->text.buffer, commandDelimiter, &commandsSavePtr);
    while (line != NULL)
    {
        // TODO: ssanf() instead of strtok_r()

        DEBUG_LOG ("line: \"%s\";", line);
        char *operandsSavePtr = NULL;

        char *command = strtok_r (line, argumentsDelimiter, &operandsSavePtr);
        DEBUG_LOG ("\tcommand: \"%s\"\n", command);
        
        bool knownCommand = 0; // for future checks
        if (command != NULL)
        {
            for (size_t i = 0; i < CommandsNumber; i++)
            {
                if (strcmp (commands[i].name, command) == 0)
                {
                    knownCommand = 1;
                    // FIXME: check for buffer overflow
                    myAsm->bytecode[bytecodeIdx] = commands[i].bytecode;
                    bytecodeIdx++;

                    char *argument = strtok_r (NULL, argumentsDelimiter, &operandsSavePtr);
                    DEBUG_LOG ("\t\t require argument: %d", commands[i].requireArgument);
                    DEBUG_LOG ("\t\t argument: \"%s\"\n", argument);

                    if (commands[i].requireArgument)
                    {
                        if (argument == NULL)
                        {
                            // TODO: please replace this shit with error codes
                            ERROR ("Missing argument on line %lu", lineNumber)

                            return ASSEMBLER_MISSING_ARGUMENT; // TODO: error_code
                        }
                        // FIXME: check for buffer overflow
                        // FIXME: check if operand is 
                        
                        // FIXME: switch between register encoding and number in push
                        DEBUG_LOG ("\t\tbytecode[bytecodeIdx - 1] = %d;", myAsm->bytecode[bytecodeIdx - 1]);
                        if (myAsm->bytecode[bytecodeIdx - 1] == SPU_PUSHR ||
                            myAsm->bytecode[bytecodeIdx - 1] == SPU_POPR)
                        {
                            myAsm->bytecode[bytecodeIdx] = argument[1] - 'A';
                        }
                        else
                        {
                            DEBUG_LOG ("\t\t %s", "atoi");
                            myAsm->bytecode[bytecodeIdx] = atoi(argument);
                        }
                        // FIXME: peace of shit

                        bytecodeIdx++; 

                        argument = strtok_r (NULL, argumentsDelimiter, &operandsSavePtr);
                        DEBUG_LOG ("\t\t\t extra argument: \"%s\"\n", argument);
                        
                        if (argument != NULL)
                        {
                            ERROR ("%sExtra argument on line %lu\n", RED_BOLD_COLOR, lineNumber)

                            return ASSEMBLER_EXTRA_ARGUMENT;
                        }
                    }
                    if (!commands[i].requireArgument && argument != NULL)
                    {
                        ERROR ("Unnecessary argument on line %lu", lineNumber) // TODO: add file name

                        return ASSEMBLER_EXTRA_ARGUMENT;
                    }

                    break;
                }
            }
            
            if (!knownCommand) 
            {
                ERROR ("Uknown command on line %lu", lineNumber)

                return ASSEMBLER_UKNOWN_COMMAND;
            }
        }

        DEBUG_LOG ("lineNumber = %lu\n", lineNumber);
        lineNumber++;
        line = strtok_r (NULL, commandDelimiter, &commandsSavePtr);
        DEBUG_LOG ("line = \"%s\"\n", line);

    }

    return ASSEMBLER_OK;
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
        DEBUG_LOG ("i = %lu;", i);

        int status = fprintf (outputFile, "%d ", myAsm->bytecode[i]);
        if (status < 0)
        {
            fprintf (stderr, "Error while writing output to %s", outputFileName);
            return status;
        }
    }

    return 0;
}

// TODO: is delimiter
size_t CountOperands (char *s, const char *delimiter)
{
    bool previousIsText = true;
    for (size_t i = 0; delimiter[i] != '\0'; i++)
    {
        if (s[0] == delimiter[i])
            previousIsText = false;
    }

    size_t counter = 0;
    for (size_t i = 0; s[i] != '\0'; i++)
    {
        bool isDelimeter = false;
        for (size_t j = 0; delimiter[j] != '\0'; j++)
        {
            if (s[i] == delimiter[j])
            {
                isDelimeter = true;
                if (previousIsText) 
                    counter++;
                break;
            }
        }
        previousIsText = !isDelimeter;
    }
    
    return counter;
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

    return 0;
}