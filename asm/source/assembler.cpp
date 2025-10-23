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

int AssembleFirst               (asm_t *myAsm);
int AssembleFinal               (asm_t *myAsm);

const command_t *FindCommand    (char **lineStart);

int AssembleLine                (asm_t *myAsm, size_t strIdx, size_t pass);
int AssembleCommand             (asm_t *myAsm, const command_t *command, 
                                 char **lineStart);
int FindArgument                (asm_t *myAsm, argument_t *argument,
                                 char **lineStart);

int AddLabel                    (asm_t *myAsm, char **lineStart);
int FindLabel                   (asm_t *myAsm, unsigned long hash);
                     
// bool IsRegisterCommand          (int commandBytecode);

int GetRegisterBytecode         (char *registerName, int *readedBytes, int *bytecode);
int GetRegisterAddressBytecode  (char *registerName, int *readedBytes, int *bytecode);

int PrintHeader                 (FILE *outputFile, asm_t *myAsm);
int PrintBytecode               (FILE *outputFile, asm_t *myAsm);

int PrintListingLine            (asm_t *myAsm, size_t instructionStart);

int AsmCtor (asm_t *myAsm)
{
    assert (myAsm);

    *myAsm = {}; // TODO: убрать лишнее

    myAsm->text.buffer = NULL;
    myAsm->text.bufferLen = 0;
    myAsm->text.lines = NULL;

    myAsm->bytecode = NULL;

    myAsm->ip = 0;
    myAsm->instructionsCnt = 16;
    myAsm->bytecode = NULL;

    myAsm->fileName = NULL;
    myAsm->lineNumber = 0;
    
    for (size_t i = 0; i < ARRAY_SIZE (myAsm->labels); i++)
    {
        myAsm->labels[i].value = -1;
        myAsm->labels[i].hash  = 0;
    }

    myAsm->fileListing = fopen ("listing.txt", "w");
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

int AllocateBytecode (asm_t *myAsm)
{
    myAsm->bytecode = (int *) calloc (myAsm->instructionsCnt, sizeof(int));

    if (myAsm->bytecode == NULL)
    {
        return MY_ASM_COMMON_ERROR |
                COMMON_ERROR_ALLOCATING_MEMORY;
    }
    
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

int FindArgument (asm_t *myAsm, argument_t *argument,
                  char **lineStart)
{
    assert(myAsm);
    assert(lineStart);
    assert(*lineStart);

    *lineStart = SkipSpaces(*lineStart);

    DEBUG_LOG ("*lineStart = \"%s\";", *lineStart);

    if (*lineStart[0] == ':')
    {
        DEBUG_LOG ("%s", "Argument is label");
        *lineStart += 1;
        DEBUG_LOG ("*lineStart = \"%s\";", *lineStart);

        argument->type = MY_ASM_ARG_LABEL;
        argument->value = -1;

        unsigned long labelHash = HashDjb2 (*lineStart, ' ');
        int idx = FindLabel (myAsm, labelHash);
        
        DEBUG_LOG ("hash: %lu;", labelHash);
        DEBUG_LOG ("idx: %d;", idx);
        
        if (idx < 0) 
            return MY_ASM_OK; // FIXME: not the best solution

        argument->value =(int) myAsm->labels[idx].value;

        return MY_ASM_OK;
    }

    int readedBytes = 0;
    int status = sscanf (*lineStart, "%d %n", &argument->value, &readedBytes);
    *lineStart += readedBytes;
    if (status == 1)
    {
        argument->type = MY_ASM_ARG_NUMBER;
        
        return MY_ASM_OK;
    }

    status = GetRegisterBytecode (*lineStart, &readedBytes, &argument->value);
    *lineStart += readedBytes;
    if (status == MY_ASM_OK)
    {
        argument->type = MY_ASM_ARG_REGISTER;

        return MY_ASM_OK;
    }

    status = GetRegisterAddressBytecode (*lineStart, &readedBytes, &argument->value);
    *lineStart += readedBytes;
    if (status == MY_ASM_OK)
    {
        argument->type = MY_ASM_ARG_REGISTER_ADDRESS;

        return MY_ASM_OK;
    }

    argument->type = MY_ASM_ARG_EMPTY;
    argument->value = ARGUMENT_DEFAULT_VALUE;

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
        return MY_ASM_OK;

    argument_t argument = {};
    DEBUG_LOG ("%s", "calls FindArgument()");
    status = FindArgument (myAsm, &argument, lineStart);
    if (status != MY_ASM_OK) 
    {
        ERROR_PRINT ("%s:%lu Missing argument for command \"%s\"", 
                     myAsm->fileName, myAsm->lineNumber, command->name);

        return status;
    }

    if (!(argument.type & command->argType))
    {
        ERROR_PRINT ("%s:%lu Wrong argument type for %s command", 
                        myAsm->fileName, myAsm->lineNumber, command->name);

        return MY_ASM_WRONG_ARGUMENT_TYPE;
    }

    DEBUG_LOG ("argument.type = %d;\n", argument.type);
    DEBUG_LOG ("argument.value = %d;\n", argument.value);

    myAsm->bytecode[myAsm->ip] = argument.value;

    myAsm->ip++;

    return MY_ASM_OK;
}


int AddLabel (asm_t *myAsm, char **lineStart)
{
    assert (lineStart);
    assert (*lineStart);

    if (*lineStart[0] != ':')
    {
        ERROR_PRINT ("%s:%lu Error reading label", 
                     myAsm->fileName, myAsm->lineNumber);

        return MY_ASM_BAD_LABEL;
    }
    *lineStart += 1;
    DEBUG_LOG ("*lineStart = \"%s\";", *lineStart);

    if (myAsm->labelIdx >= SPU_MAX_LABELS_COUNT)
    {
        ERROR_PRINT ("%s:%lu Error adding label, max number of labels is %lu",
                     myAsm->fileName, myAsm->lineNumber, SPU_MAX_LABELS_COUNT);
    
        return MY_ASM_BAD_LABEL;
    }

    unsigned long labelHash = HashDjb2 (*lineStart, ' ');

    DEBUG_LOG ("labelHash = %lu;", labelHash);

    int idx = FindLabel (myAsm, labelHash);

    if (idx >= 0)
    {
        // TODO: fix collision
        myAsm->labels[idx].hash = labelHash;
        myAsm->labels[idx].value = (ssize_t)myAsm->ip;
    }
    else
    {
        myAsm->labels[myAsm->labelIdx].hash = labelHash;
        myAsm->labels[myAsm->labelIdx].value = (ssize_t)myAsm->ip;

        myAsm->labelIdx++;
    }

    DEBUG_PRINT ("myAsm->labels[%lu]:\n", ARRAY_SIZE(myAsm->labels));
    for (size_t i = 0; i < ARRAY_SIZE (myAsm->labels); i++)
    {
        DEBUG_PRINT ("\tmyAsm->labels[%lu].hash = %lu;\n", 
                     i, myAsm->labels[i].hash);
        DEBUG_PRINT ("\tmyAsm->labels[%lu].value = %zd;\n", 
                     i, myAsm->labels[i].value);
    }

    return MY_ASM_OK;
}

int FindLabel (asm_t *myAsm, unsigned long hash)
{
    int idx = -1;

    for (size_t i = 0; i < ARRAY_SIZE (myAsm->labels); i++)
    {
        if (myAsm->labels[i].hash == hash)
        {
            idx = (int)i;
            break;
        }
    }

    return idx;
}

const command_t *FindCommand (char **lineStart)
{
    size_t wordLen = GetWordLen (*lineStart, " ");
    
    DEBUG_LOG ("wordLen = %lu;", wordLen);

    for (size_t i = 0; i < CommandsNumber; i++)
    {
        if (wordLen != commands[i].nameLen - 1) continue;

        if (strncmp (*lineStart, commands[i].name, wordLen) == 0)
        {
            *lineStart += commands[i].nameLen;

            return &(commands[i]);
        }
    }

    return NULL;
}

int AssembleLine (asm_t *myAsm, size_t strIdx, size_t pass)
{
    DEBUG_PRINT ("%s", "\n\n");

    DEBUG_PRINT ("[%lu].len: %lu\n", 
                 strIdx, myAsm->text.lines[strIdx].len);
    DEBUG_PRINT ("[%lu].start: %p\n", 
                 strIdx, myAsm->text.lines[strIdx].start);
    DEBUG_PRINT ("[%lu] = \"%.*s\"\n",  
                 strIdx, (int) myAsm->text.lines[strIdx].len - 1, myAsm->text.lines[strIdx].start);

    DEBUG_LOG ("myAsm->ip = %lu;", myAsm->ip);
    
    myAsm->lineNumber = strIdx + 1;

    char *lineStart = myAsm->text.lines[strIdx].start;
          lineStart = SkipSpaces (lineStart);

    if (lineStart[0] == '\0') 
        return MY_ASM_OK;
    
    // TODO: new function TryFindLabel() (like FindCommand())
    if (lineStart[0] == ':')
    {
        DEBUG_LOG ("%s", "calls AddLabel()");

        int status = AddLabel (myAsm, &lineStart);

        // FIXME: check for trash symbols

        return status;
    }
    
    const command_t *command = FindCommand (&lineStart);
    if (command == NULL) 
    {
        ERROR_PRINT ("%s:%lu Uknown command \"%s\"", myAsm->fileName, myAsm->lineNumber, lineStart);

        return MY_ASM_UKNOWN_COMMAND;
    }
    DEBUG_LOG ("command bytecode = %d;", command->bytecode);

    if (pass == MY_ASM_FIRST_PASS)
    {
        myAsm->ip += 1;
        myAsm->ip += (command->argType != MY_ASM_ARG_EMPTY); 
        // maybe not the best solution, but it works
        
        return MY_ASM_OK;
    }
    
    const size_t instructionStart = myAsm->ip;

    int status = AssembleCommand (myAsm, command, &lineStart);
    if (status != MY_ASM_OK) 
        return status;
    
    status = PrintListingLine (myAsm, instructionStart);
    if (status != MY_ASM_OK)
        return status;

    // FIXME: CheckTrash();

    return MY_ASM_OK;
}

int Assemble (asm_t *myAsm)
{
    int status = AssembleFirst (myAsm);
    if (status != 0)
    {
        ERROR ("%s", "Error in first assembling")

        return status;
    }

    status = AllocateBytecode (myAsm);
    if (status != 0)
    {
        ERROR ("%s", "Error in allocating bytecode")
        
        return status;
    }

    status = AssembleFinal (myAsm);
    if (status != 0)
    {
        ERROR ("%s", "Error in second(final) assembling")
        
        return status;
    }

    return MY_ASM_OK;
}

int AssembleFirst (asm_t *myAsm)
{
    assert (myAsm);

    DEBUG_PRINT ("%s", "==========================================================\n");
    DEBUG_PRINT ("%s", "                   ASSEMBLE - FIRST PASS!                 \n");
    DEBUG_PRINT ("%s", "==========================================================\n");

    for (size_t i = 0; myAsm->text.lines[i].start != 0; i++)
    {
        int status = AssembleLine (myAsm, i, MY_ASM_FIRST_PASS);
        if (status != MY_ASM_OK)
            return status;
    }

    myAsm->instructionsCnt += myAsm->ip + 1;

    return MY_ASM_OK;
}
int AssembleFinal (asm_t *myAsm)
{
    assert (myAsm);

    DEBUG_PRINT ("%s", "==========================================================\n");
    DEBUG_PRINT ("%s", "                   ASSEMBLE - FINAL PASS!                 \n");
    DEBUG_PRINT ("%s", "==========================================================\n");

    myAsm->ip = 0;

    DEBUG_LOG ("Allocated bytecode array on %lu elements", myAsm->instructionsCnt);

    for (size_t i = 0; myAsm->text.lines[i].start != 0; i++)
    {
        int status = AssembleLine (myAsm, i, MY_ASM_FINAL_PASS);
        if (status != MY_ASM_OK)
            return status;
    }

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

    DEBUG_LOG("%s", "PrintBinary() is OK");
    
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
        

        if (status < 0)   
            return COMMON_ERROR_WRITE_TO_FILE;
    }

    return MY_ASM_OK;
}

int PrintListingLine (asm_t *myAsm, size_t instructionStart)
{
    // TODO: add labels definitios to listring file
    int status = fprintf (myAsm->fileListing, "%04lu \t ", instructionStart);
    if (status < 0)
        return COMMON_ERROR_WRITE_TO_FILE;

    size_t spaceAlign = 4 + 1 + 4 + 1;
    for (size_t i = instructionStart; i < myAsm->ip; i++)
    {
        status = fprintf (myAsm->fileListing, "%4d ", myAsm->bytecode[i]);
        if (status < 0)
            return COMMON_ERROR_WRITE_TO_FILE;
        spaceAlign -= (size_t) status;
    }

    DEBUG_LOG ("spaceAlign = %lu;", spaceAlign); 
    // FIXME: printf("%20s")

    status = PrintSymbols (myAsm->fileListing, spaceAlign, ' ');
    if (status != MY_ASM_OK)
        return status;
    
    // NOTE: maybe better to pass here command_t and argument_t
    // but I do not think converting from int to register name, label and other types is worth it
    status = fprintf (myAsm->fileListing, "\t %s\n", 
                          myAsm->text.lines[myAsm->lineNumber - 1].start);
    if (status < 0)
        return COMMON_ERROR_WRITE_TO_FILE;

    return MY_ASM_OK;
}

// bool IsRegisterCommand (int commandBytecode)
// {
//     const int registerBit = 1 << SHIFT_REGISTER;

//     return (commandBytecode & registerBit);
// }


// Yes, you can't do [ RAX ], only [RAX]
int GetRegisterAddressBytecode (char *registerName, int *readedBytes, int *bytecode)
{
    assert (registerName);
    assert (readedBytes);
    assert (bytecode);

    if (strlen(registerName) < REGISTER_NAME_LEN + 2) return MY_ASM_INVALID_REGISTER_NAME;
    
    if (registerName[0] != '[') return MY_ASM_INVALID_REGISTER_NAME;
    if (registerName[4] != ']') return MY_ASM_INVALID_REGISTER_NAME;

    int status = GetRegisterBytecode (registerName + 1, readedBytes, bytecode);
    if (status != MY_ASM_OK)
        return status;

    readedBytes += 2;

    return MY_ASM_OK;
}
int GetRegisterBytecode (char *registerName, int *readedBytes, int *bytecode)
{
    if (strlen(registerName) < REGISTER_NAME_LEN) return MY_ASM_INVALID_REGISTER_NAME;

    if (registerName[0] != 'R') return MY_ASM_INVALID_REGISTER_NAME;
    if (registerName[2] != 'X') return MY_ASM_INVALID_REGISTER_NAME;

    int registerIdx = registerName[1] - 'A';

    if (registerIdx < 0)                            return MY_ASM_INVALID_REGISTER_NAME;
    if ((size_t)registerIdx >= NUMBER_OF_REGISTERS) return MY_ASM_INVALID_REGISTER_NAME;

    *bytecode = registerIdx;
    *readedBytes = REGISTER_NAME_LEN;

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

    free (myAsm->bytecode);
    myAsm->bytecode = NULL;

    myAsm->instructionsCnt = 0;

    myAsm->fileName   = NULL;
    myAsm->lineNumber = 0;

    for (size_t i = 0; i < ARRAY_SIZE (myAsm->labels); i++)
    {
        myAsm->labels[i].value = 0;
    }

    fclose (myAsm->fileListing);

    return MY_ASM_OK;
}