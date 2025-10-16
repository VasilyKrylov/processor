#include <assert.h>
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

// TODO:
enum token_type
{
    LABEL = 0,
    OPERAND = 1,
    REGISTER = 2,
    NEW_LINE = 3,

};
struct token
{
    token_type type;
    int data;

    size_t lineNumber;
};

int ResizeBytecode (asm_t *myAsm);

int AssembleCommand         (asm_t *myAsm, const command_t *command, 
                             char **lineStart);
int AssembleRegisterCommand (asm_t *myAsm, const command_t *command, int *argumentBytecode,
                             char **lineStart);
int AssembleJumpLabelCommand (asm_t *myAsm, const command_t *command, int *argumentBytecode,
                              char **lineStart);

int AddLabel (asm_t *myAsm, char **lineStart);
int GetLabelName (int labelIdx);
int GetLabelIdx (char c);
int CheckLabel (asm_t *myAsm, int labelIdx);
                     
bool IsRegisterCommand (int commandBytecode);
bool IsJumpCommand (int commandBytecode);

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
    // TODO: macro COMMAND (IN, 0)
};

static const size_t CommandsNumber = sizeof(commands) / sizeof(command_t);

int AsmCtor (asm_t *myAsm)
{
    assert (myAsm);

    myAsm->text.buffer = NULL;
    myAsm->text.bufferLen = 0;
    myAsm->text.lines = NULL;

    myAsm->bytecode = NULL;

    myAsm->ip = 0;
    myAsm->instructionsCnt = 0;

    myAsm->fileName = NULL;
    myAsm->lineNumber = 0;

    for (size_t i = 0; i < ARRAY_SIZE (myAsm->labels); i++)
    {
        myAsm->labels[i] = -1;
    }

    myAsm->fileListing = fopen ("my_asm.listing", "w");
    
    if (myAsm->fileListing == NULL)
    {
        perror ("Error opening output file for assembler listing");

        return MY_ASM_COMMON_ERROR | 
               COMMON_ERROR_OPENING_FILE;
               // TODO: print error for MY_ASM_COMMON_ERROR
    }

    return MY_ASM_OK;
}

int AsmRead (asm_t *myAsm, char *inputFileName) // FIXME asserts
{
    DEBUG_LOG ("%s", "AsmRead()");

    int status = TextCtor (inputFileName, &(myAsm->text));

    DEBUG_LOG ("buffer: \n\"%s\"\n", myAsm->text.buffer);
    
    if (status != 0)
    {
        return 1;
    }
    
    myAsm->fileName = inputFileName;
    myAsm->instructionsCnt = 8; // FIXME const
    myAsm->bytecode = (int *) calloc (myAsm->instructionsCnt, sizeof(int));

    DEBUG_LOG ("myAsm->instructionsCnt: %lu\n", myAsm->instructionsCnt);
    
    StrReplace (myAsm->text.buffer, "\n;", '\0');

    return MY_ASM_OK;
}

int ResizeBytecode (asm_t *myAsm)
{
    // + 1 beacuse we check space for current command and potential argument

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

    return MY_ASM_OK;
}

int AssembleJumpLabelCommand (asm_t *myAsm, const command_t *command, int *argumentBytecode,
                              char **lineStart)
{
    DEBUG_LOG ("Jump family command with label argument: %s\n", command->name);

    *lineStart += 1; // skip space, go to argument

    char labelName = *lineStart[0]; // get argument
    int labelIdx = GetLabelIdx (labelName);

    *lineStart += 1; // skip argument, end of str

    DEBUG_LOG ("*lineStart = \'%c\';", *lineStart[0]);
    DEBUG_LOG ("labelName = \'%c\';", labelName);

    DEBUG_LOG ("GetLabelIdx(labelName) = %d\n", GetLabelIdx(labelName));
    DEBUG_LOG ("myAsm->labels[GetLabelIdx(labelName)] = %d;\n", myAsm->labels[GetLabelIdx(labelName)]);
        
    int result = CheckLabel (myAsm, labelIdx);
    if (result != MY_ASM_OK) 
        return result;

    *argumentBytecode = myAsm->labels[labelIdx];

    return MY_ASM_OK;
}

int AssembleRegisterCommand (asm_t *myAsm, const command_t *command, int *argumentBytecode,
                             char **lineStart)
{
    DEBUG_LOG ("Register family command: %s\n", command->name);

    char regStr[8] = {}; // TODO: it should be 4, but i recive
    // warning: stack protector not protecting function: all local arrays are less than 8 bytes long [-Wstack-protector]

    int readedBytes = 0;
    int result = sscanf (*lineStart, "%3s %n", regStr, &readedBytes); 
    *lineStart += readedBytes;

    if (result != 1)
    {
        ERROR_PRINT ("%s:%lu Error reading register name, whish is required for %s command\n",
                     myAsm->fileName, myAsm->lineNumber, command->name)
        
        return MY_ASM_INVALID_REGISTER_NAME;
    }

    result = GetRegisterBytecode (regStr, argumentBytecode);
    if (result != MY_ASM_OK)
    {
        ERROR_PRINT ("%s:%lu Invalid register name. Correct one for exammple is \"RAX\"\n",
                     myAsm->fileName, myAsm->lineNumber)

        return result;
    }

    return MY_ASM_OK;
}

// TODO: check for this errors:
// ADD 10
// PUSH 20 20
// PUSHR RAX RBX
// TODO: make functions:  AssembleArgument
//                        Resize
int AssembleCommand (asm_t *myAsm, const command_t *command, 
                     char **lineStart)
{
    assert(myAsm);
    assert(lineStart);
    assert(*lineStart);

    int result = ResizeBytecode (myAsm);

    if (result != MY_ASM_OK)
        return result;

    myAsm->bytecode[myAsm->ip] = command->bytecode;

    myAsm->ip++;
    
    if (command->requireArgument)
    {
        // TODO: new fuction - AssembleArgument
        int argumentBytecode = -666;
        
        int readedBytes = 0;

        if (IsRegisterCommand (command->bytecode)) 
        {
            result = AssembleRegisterCommand (myAsm, command, &argumentBytecode, lineStart);
            if (result != MY_ASM_OK)
                return result;
        }

        else if (IsJumpCommand (command->bytecode) && *lineStart[0] == ':')
        {
            result = AssembleJumpLabelCommand (myAsm, command, &argumentBytecode, lineStart);
            if (result != MY_ASM_OK)
                return result;
        }

        else
        {
            result = sscanf (*lineStart, "%d %n", &argumentBytecode, &readedBytes);
            *lineStart += readedBytes;

            if (result != 1)
            {
                ERROR_PRINT ("%s:%lu Error reading command argument, whish is required for %s command\n",
                             myAsm->fileName, myAsm->lineNumber, command->name)

                return MY_ASM_MISSING_ARGUMENT;
            }
        }


        DEBUG_PRINT ("argumentBytecode = %d;\n", argumentBytecode);

        myAsm->bytecode[myAsm->ip] = argumentBytecode;

        myAsm->ip++;
    }

    // FIXME: just go over linesStart checking for not probel
    char trash = '\0';
    int res = sscanf (*lineStart, "%c", &trash);

    if (res == 1)
    {
        ERROR_PRINT ("%s:%lu Trash symbols after command %s, \'%c\'=%d\n",
                        myAsm->fileName, myAsm->lineNumber, command->name, trash, trash)

        return MY_ASM_TRASH_SYMBOLS;
    }

    //check for trash symbols

    return MY_ASM_OK;
}

int AddLabel (asm_t *myAsm, char **lineStart)
{
    assert (lineStart);
    assert (*lineStart);

    DEBUG_LOG ("*lineStart = '%c', %d;", **lineStart, **lineStart);
    DEBUG_LOG ("*(lineStart+1) = '%c', %d;", *(*lineStart + 1), *(*lineStart + 1));
    DEBUG_LOG ("*(lineStart+2) = '%c', %d;", *(*lineStart + 2), *(*lineStart + 2));

    *lineStart += 1; // go to the argument
    char labelName = *lineStart[0];
    *lineStart += 1; // end of the string

    int labelIdx = GetLabelIdx(labelName);

    int result = CheckLabel (myAsm, labelIdx);
    if (result != MY_ASM_OK) 
        return result;

    if (myAsm->labels[labelIdx] != (ssize_t)myAsm->ip)
    {
        ERROR_PRINT ("%s:%lu Double assigning label with name %c", 
                     myAsm->fileName, myAsm->lineNumber, labelIdx + '0');

        ERROR_PRINT ("Old label value is = %d, new label value = %lu", 
                     myAsm->labels[labelIdx], myAsm->ip);

        ERROR_PRINT ("%s", "");

        return MY_ASM_LABEL_DOUBLE_ASSIGNMENT;
    }

    DEBUG_PRINT ("myAsm->labels[%d] = %d;\n", labelIdx, myAsm->labels[labelIdx]);
    
    myAsm->labels[labelIdx] = (ssize_t)myAsm->ip;

    // while (*lineStart[0] != '\0')
    // {
    //     char c = *lineStart[0];

    //     if (c != ' ')
    //     {
    //         ERROR_PRINT ("%s:%lu Trash symbol %c after label %d", 
    //                     myAsm->fileName, myAsm->lineNumber, c, labelIdx);

    //         return MY_ASM_TRASH_SYMBOLS;
    //     }

    //     *lineStart += 1;
    // }
    char trash = '\0';
    int res = sscanf (*lineStart, "%c", &trash);
    if (res == 1) 
    {
        ERROR_PRINT ("%s:%lu Trash symbol %c after label %d", 
                     myAsm->fileName, myAsm->lineNumber, trash, labelIdx);

        return MY_ASM_TRASH_SYMBOLS;
    }

    return MY_ASM_OK;
}

int Assemble (asm_t *myAsm)
{
    myAsm->ip = 0;

    for (size_t i = 0; myAsm->text.lines[i].start != 0; i++)
    {
        myAsm->lineNumber = i + 1;
        
        DEBUG_PRINT ("[%lu].len: %lu\n", i, myAsm->text.lines[i].len);
        DEBUG_PRINT ("[%lu].start: %p\n", i, myAsm->text.lines[i].start);
        DEBUG_PRINT ("[%lu] = \"%.*s\"\n", i, (int) myAsm->text.lines[i].len - 1, myAsm->text.lines[i].start);

        char *lineStart = myAsm->text.lines[i].start;

        // TODO: new function token processing
        if (lineStart[0] == '\0') 
            continue;
        
        if (lineStart[0] == ':')
        {
            int status = AddLabel (myAsm, &lineStart);

            if (status == MY_ASM_OK)
                continue;
            else
                return status;
        }
            
        bool knownCommand = false;
        int readedBytes = 0;
        char command[MAX_COMMAND_LEN] = {}; // FIXME: constant for command!!!!!!!!!!!

        // FIXME: just use strncmp bro and make function to skip spaces (strip)
        sscanf (lineStart, "%s %n", command, &readedBytes);
        
        lineStart += readedBytes;

        DEBUG_PRINT ("command = \"%s\", readedBytes = %d;\n", command, readedBytes);
        
        // TODO: make this for-loop in another function
        for (size_t j = 0; j < CommandsNumber; j++)
        {
            if (strcmp (commands[j].name, command) == 0)
            {
                knownCommand = true;

                int res = AssembleCommand (myAsm, &commands[j], &lineStart);

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

    return MY_ASM_OK;
}

bool IsRegisterCommand (int commandBytecode)
{
    const int registerBit = 1 << 5;

    return (commandBytecode & registerBit);
}
bool IsJumpCommand (int commandBytecode)
{
    const int registerBit = 1 << 6; //FIXME: CONSTANT

    return (commandBytecode & registerBit);
}

int GetLabelIdx (char labelName)
{
    return labelName - '0';
}

int GetLabelName (int labelIdx)
{
    return labelIdx + '0';
}

int CheckLabel (asm_t *myAsm, int labelIdx)
{
    if (labelIdx < 0 || (size_t)labelIdx > ARRAY_SIZE (myAsm->labels)) 
    {
        ERROR_PRINT ("%s:%lu Bad label name, only [0-9] allowed, but you typed %c",
                     myAsm->fileName, myAsm->lineNumber, GetLabelName(labelIdx));

        return MY_ASM_BAD_LABEL_NAME;
    }

    return MY_ASM_OK;
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

    if (error & MY_ASM_UKNOWN_COMMAND)          fprintf(stderr, "%s", "Uknown command in input file\n");
    if (error & MY_ASM_MISSING_ARGUMENT)        fprintf(stderr, "%s", "Missing argument for command that requires it\n");
    if (error & MY_ASM_TRASH_SYMBOLS)           fprintf(stderr, "%s", "Trash symbols after completed command\n");
    if (error & MY_ASM_INVALID_REGISTER_NAME)   fprintf(stderr, "%s", "Invalid register name. Correct one for exammple is \"RAX\"\n");
    if (error & MY_ASM_LABEL_DOUBLE_ASSIGNMENT) fprintf(stderr, "%s", "Double label assignment\n");

    fprintf(stderr, "%s", COLOR_END);
    
    return MY_ASM_OK;
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

    myAsm->fileName   = NULL;
    myAsm->lineNumber = 0;

    for (size_t i = 0; i < ARRAY_SIZE (myAsm->labels); i++)
    {
        myAsm->labels[i] = 0;
    }

    fclose (myAsm->fileListing);

    return MY_ASM_OK;
}