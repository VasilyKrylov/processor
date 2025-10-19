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

int ResizeBytecode              (asm_t *myAsm);

const command_t *FindCommand    (char **lineStart);
int AssembleCommand             (asm_t *myAsm, const command_t *command, 
                                 char **lineStart);
int AssembleArgument            (asm_t *myAsm, argument_t *argument,
                                 char **lineStart);

int AddLabel                    (asm_t *myAsm, char **lineStart);
int GetLabelName                (int labelIdx);
int GetLabelIdx                 (char c);
int CheckLabel                  (asm_t *myAsm, int labelIdx);
                     
bool IsRegisterCommand          (int commandBytecode);
bool IsJumpCommand              (int commandBytecode);

int GetRegisterBytecode         (char *registerName, int *bytecode);

int PrintHeader                 (FILE *outputFile, asm_t *myAsm);
int PrintBytecode               (FILE *outputFile, asm_t *myAsm);

// TODO: change argType to int

int AsmCtor (asm_t *myAsm)
{
    assert (myAsm);

    *myAsm = {}; // TODO: убрать лишнее

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
    myAsm->instructionsCnt = 0;

    DEBUG_LOG ("myAsm->instructionsCnt: %lu\n", myAsm->instructionsCnt);
    
    StrReplace (myAsm->text.buffer, "\n;", '\0');

    return MY_ASM_OK;
}

int ResizeBytecode (asm_t *myAsm)
{
    // + 1 beacuse we check space for current command and potential argument

    if (myAsm->ip + 1 >= myAsm->instructionsCnt)
    {
        ERROR ("%s", 
            "ResizeBytecode() is going to resize bytecode array \n"
            "\t\tThis should never be executed, because size of array precalculated on first pass");

        myAsm->instructionsCnt *= 2;

        int *newBytecode = (int *) realloc (myAsm->bytecode, sizeof(int) * myAsm->instructionsCnt); // TODO: memset()?

        if (newBytecode == NULL)
        {
            perror ("Error trying to realloc myAsm->bytecode");

            return MY_ASM_COMMON_ERROR |
                   COMMON_ERROR_REALLOCATING_MEMORY;
        }

        myAsm->bytecode = newBytecode;
        
        ERROR ("Bytecode reallocated to %lu number of elemenets", myAsm->instructionsCnt);
    }


    return MY_ASM_OK;
}

int AssembleArgument (asm_t *myAsm, argument_t *argument,
                      char **lineStart)
{
    assert(myAsm);
    assert(lineStart);
    assert(*lineStart);

    int readedBytes = 0;
    int status = sscanf (*lineStart, ":%d %n", &argument->value, &readedBytes);
    if (status == 1)
    {
        argument->type = MY_ASM_ARG_LABEL;

        int result = CheckLabel (myAsm, argument->value);
        if (result != MY_ASM_OK)
            return result;

        return MY_ASM_OK;
    }

    status = sscanf (*lineStart, "%d", &argument->value);
    if (status == 1)
    {
        argument->type = MY_ASM_ARG_NUMBER;
        
        return MY_ASM_OK;
    }

    status = GetRegisterBytecode (*lineStart, &argument->value);
    if (status == MY_ASM_OK)
    {
        argument->type = MY_ASM_ARG_REGISTER;

        return MY_ASM_OK;
    }

    return MY_ASM_MISSING_ARGUMENT;
}

// TODO: check for this errors:
// ADD 10
// PUSH 20 20
// PUSHR RAX RBX
//                        Resize
int AssembleCommand (asm_t *myAsm, const command_t *command, 
                     char **lineStart)
{
    assert(myAsm);
    assert(command);
    assert(lineStart);
    assert(*lineStart);

    int status = ResizeBytecode (myAsm);
    if (status != MY_ASM_OK)
        return status;

    myAsm->bytecode[myAsm->ip] = command->bytecode;

    myAsm->ip++;
    
    if (command->argType == MY_ASM_ARG_EMPTY)
    {
        return MY_ASM_OK;
    }

    argument_t argument = {};
    status = AssembleArgument (myAsm, &argument, lineStart);

    if (status != MY_ASM_OK)
    {
        ERROR_PRINT ("%s:%lu Error assembling %s argument", 
                        myAsm->fileName, myAsm->lineNumber, command->name);

        return MY_ASM_MISSING_ARGUMENT;
    }

    if (!(argument.type & command->argType))
    {
        ERROR_PRINT ("%s:%lu Wrong argument type for %s command", 
                        myAsm->fileName, myAsm->lineNumber, command->name);

        return MY_ASM_WRONG_ARGUMENT_TYPE;
    }

    DEBUG_PRINT ("argument.type = %d;\n", argument.type);
    DEBUG_PRINT ("argument.value = %d;\n", argument.value);

    myAsm->bytecode[myAsm->ip] = argument.value;

    myAsm->ip++;

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

    int labelIdx = GetLabelIdx (labelName);

    int result = CheckLabel (myAsm, labelIdx);
    if (result != MY_ASM_OK) 
        return result;

    if (myAsm->labels[labelIdx] != LABEL_DEFAULT_VALUE &&
    myAsm->labels[labelIdx] != (ssize_t)myAsm->ip)
    {
        ERROR_PRINT ("%s:%lu Double assigning label with name %c", 
                     myAsm->fileName, myAsm->lineNumber, labelIdx + '0');

        ERROR_PRINT ("Old label value is = %zd, new label value = %lu", 
                     myAsm->labels[labelIdx], myAsm->ip);

        ERROR_PRINT ("%s", "");

        return MY_ASM_LABEL_DOUBLE_ASSIGNMENT;
    }

    DEBUG_PRINT ("%s", "Before assignment:\n");
    DEBUG_PRINT ("myAsm->labels[%d] = %zd;\n", labelIdx, myAsm->labels[labelIdx]);
    
    myAsm->labels[labelIdx] = (ssize_t)myAsm->ip;

    DEBUG_PRINT ("%s", "After assignment:\n");
    DEBUG_PRINT ("myAsm->labels[%d] = %zd;\n", labelIdx, myAsm->labels[labelIdx]);

    // FIXME: check for trash symbols

    return MY_ASM_OK;
}

const command_t *FindCommand (char **lineStart)
{
    size_t wordLen = GetWordLen (*lineStart, ' ');
    
    DEBUG_LOG ("wordLen = %lu;", wordLen);

    for (size_t i = 0; i < CommandsNumber; i++)
    {
        // FIXME: skip if len doesn't match

        if (strncmp (*lineStart, commands[i].name, wordLen) == 0)
        {
            *lineStart += commands[i].nameLen;

            return &(commands[i]);
        }
    }

    return NULL;
}

int Assemble (asm_t *myAsm, size_t pass)
{
    assert (myAsm);

    DEBUG_PRINT ("%s", "\n");
    DEBUG_PRINT ("%s", "==========================================================\n");
    DEBUG_PRINT ("\t\t ASSEMBLE - %lu PASS!\n", pass);
    DEBUG_PRINT ("%s", "==========================================================\n");
    DEBUG_PRINT ("%s", "\n");

    myAsm->ip = 0;
    if (pass == 2)
    {
        myAsm->bytecode = (int *) calloc (myAsm->instructionsCnt, sizeof(int));

        if (myAsm->bytecode == NULL)
            return MY_ASM_COMMON_ERROR |
                   COMMON_ERROR_ALLOCATING_MEMORY;

        DEBUG_LOG ("Allocated bytecode array on %lu elements", myAsm->instructionsCnt);
    }

    for (size_t i = 0; myAsm->text.lines[i].start != 0; i++)
    {
        DEBUG_PRINT ("%s", "\n\n");

        // TODO: new function

        myAsm->lineNumber = i + 1;
        
        DEBUG_PRINT ("[%lu].len: %lu\n", i, myAsm->text.lines[i].len);
        DEBUG_PRINT ("[%lu].start: %p\n", i, myAsm->text.lines[i].start);
        DEBUG_PRINT ("[%lu] = \"%.*s\"\n", i, (int) myAsm->text.lines[i].len - 1, myAsm->text.lines[i].start);
        DEBUG_LOG ("myAsm->ip = %lu;",   myAsm->ip);

        char *lineStart = myAsm->text.lines[i].start;

        // TODO:  add strip()

        // TODO: new function token processing
        if (lineStart[0] == '\0') 
            continue;
        
        // TODO: new function TryFindLabel() (like FindCommand())
        if (lineStart[0] == ':')
        {
            int status = AddLabel (myAsm, &lineStart);

            if (status == MY_ASM_OK)
                continue;
            else
                return status;
        }
        
        const command_t *command = FindCommand (&lineStart);
        if (command == NULL) 
        {
            // FIMXME: command
            ERROR_PRINT ("%s:%lu Uknown command \"%s\"", myAsm->fileName, myAsm->lineNumber, lineStart);

            return MY_ASM_UKNOWN_COMMAND;
        }
        DEBUG_LOG ("command bytecode = %d;", command->bytecode);

        if (pass == 1)
        {
            myAsm->ip += 1;
            myAsm->ip += (command->argType != MY_ASM_ARG_EMPTY);
        }
        else if (pass == 2)
        {
            int res = AssembleCommand (myAsm, command, &lineStart);
            if (res != MY_ASM_OK) 
                return res;
        }

        // FIXME: CheckTrash();
    }

    if (pass == 1)
        myAsm->instructionsCnt += myAsm->ip + 1;

    return MY_ASM_OK;
}

int PrintBinary (const char *outputFileName, asm_t *myAsm)
{
    FILE *outputFile = fopen (outputFileName, "w");
    if (outputFile == NULL)
    {
        perror ("Error opening output file");
        return MY_ASM_COMMON_ERROR |
               COMMON_ERROR_OPENING_FILE;
    }
    
    int status = PrintHeader (outputFile, myAsm);
    if (status != MY_ASM_OK)
    {
        ERROR_PRINT ("Error while writing output to %s", outputFileName);
    }

    status = PrintBytecode (outputFile, myAsm);
    if (status != MY_ASM_OK)
    {
        ERROR_PRINT ("Error while writing output to %s", outputFileName);
    }

    return MY_ASM_OK;
}

int PrintHeader (FILE *outputFile, asm_t *myAsm)
{
    int status = fprintf (outputFile, "%lu ", MY_ASM_VERSION);
    if (status < 0)
        return COMMON_ERROR_WRITE_TO_FILE;

    status = fprintf (outputFile, "%lu ", myAsm->instructionsCnt);
    if (status < 0)
        return COMMON_ERROR_WRITE_TO_FILE;

    return MY_ASM_OK;
}
int PrintBytecode (FILE *outputFile, asm_t *myAsm)
{
    DEBUG_LOG("myAsm->instructionsCnt = %lu;\n", myAsm->instructionsCnt);

    for (size_t i = 0; i < myAsm->instructionsCnt; i++)
    {
        int status = fprintf (outputFile, "%d ", myAsm->bytecode[i]);
        
        DEBUG_LOG("i = %lu;", i);

        if (status < 0)   
            return COMMON_ERROR_WRITE_TO_FILE;
    }

    return MY_ASM_OK;
}

bool IsRegisterCommand (int commandBytecode)
{
    const int registerBit = 1 << SHIFT_REGISTER;

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
    if (strlen(registerName) < 3) return MY_ASM_INVALID_REGISTER_NAME;

    if (registerName[0] != 'R') return MY_ASM_INVALID_REGISTER_NAME;
    if (registerName[2] != 'X') return MY_ASM_INVALID_REGISTER_NAME;

    int registerIdx = registerName[1] - 'A';

    if (registerIdx < 0)                            return MY_ASM_INVALID_REGISTER_NAME;
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

    if (error & MY_ASM_COMMON_ERROR)            fprintf(stderr, "%s", "TODO: print common error function\n");

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