#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file.h"

#include "debug.h"

// DEBUG
#ifdef PRINT_DEBUG
void PrintContent (char *content)
{
    DEBUG_PRINT ("%s", "\ncontent:\n");
    for (size_t i = 0; content[i] != '\0'; i++)
    {
        DEBUG_PRINT ("[%lu] = \t %d, \t '%c'\n", i, content[i], content[i]);
    }
}

void PrintLinesArray (line *linesArray)
{
    DEBUG_PRINT ("%s", "\nlinesArray:\n");
    for (size_t i = 0; linesArray[i].start != NULL; i++)
    {
        DEBUG_PRINT ("[%lu].len: %lu\n", i, linesArray[i].len);
        DEBUG_PRINT ("[%lu].start: %p\n", i, linesArray[i].start);
        DEBUG_PRINT ("[%lu] relative start: %ld\n", i, linesArray[i].start - linesArray[0].start);

        DEBUG_PRINT ("[%lu]: ", i);
        for (size_t j = 0; j < linesArray[i].len; ++j)
        {
            DEBUG_PRINT ("\t [%lu]: '%c'\n", j, *(linesArray[i].start + j));
        }
        DEBUG_PRINT ("%s", "\n");
    }
}
#endif // PRINT_DEBUG


// creat content buffer fileSize + 2
// content[fileSize] = '\n' - looks like shit tbh
// content[fileSize + 1] = '\0'
char *ReadFile (char *inputFileName, size_t *bufferLen)
{
    FILE *inputFile = fopen (inputFileName, "r");
    if (inputFile == NULL)
    {
        ERROR ("Error opening input file \"%s\"", inputFileName);
        
        return NULL;
    }

    // FIXME: converting ssize_t to size_t in lot of places
    ssize_t fileSize = GetFileSize (inputFileName);
    if (fileSize == -1)
    {
        return NULL;
    }

    char *content = (char *) calloc ((size_t)fileSize + 2, sizeof(char));

    if (content == NULL)
    {
        perror ("Failed to allocate memory for text");

        fclose (inputFile);

        return NULL;
    }

    size_t bytesRead = fread (content, 1, (size_t)fileSize, inputFile);
    fclose (inputFile);

    if ((ssize_t)bytesRead != fileSize)
    {
        ERROR ("fread() status code(how many bytes read) is: %lu", bytesRead)
        ERROR ("fileSize is: %ld", fileSize)

        free (content);

        return NULL;
    }

    *bufferLen = (size_t) fileSize + 2;

    if (content[fileSize - 1] != '\n') 
    {
        content[fileSize] = '\n';
    }

    return content;
}

ssize_t GetFileSize (char *fileName)
{
    struct stat fileStat;

    int status = stat (fileName, &fileStat);
    if (status != 0)
    {
        perror ("Error getting file size");
        return -1;
    }
    
    return fileStat.st_size;
}

// innitializing linesArray array
// return number of initialized pointers
line *MakePointers (char *buffer, char lineSeparator)
{
    assert (buffer);

    size_t curIdx = 0;
    line *linesArray = (line *) calloc (CountNotEmptyLines (buffer, lineSeparator) + 1, sizeof(line));
    
    if (linesArray == NULL) 
    {
        perror ("Error allocating memory for linesArray");
        return NULL;
    }

    linesArray[0].start = buffer;

    for (size_t i = 0; buffer[i] != '\0'; i++)
    {
        if (buffer[i] == lineSeparator)
        {
            if (curIdx == 0)
            {
                linesArray[curIdx].len = i + 1;
            }
            else
            {
                const line prev = linesArray[curIdx - 1];
                linesArray[curIdx].len = size_t (&buffer[i] - (prev.start + prev.len) + 1);
            }

            linesArray[curIdx + 1].start = &buffer[i] + sizeof(char);

            curIdx++;
        }
    }
    linesArray[curIdx].start = NULL;

    return linesArray;
}

// counts lines using lineSeparator
size_t CountLines (char *content, char lineSeparator)
{
    assert(content);

    size_t lines = 0;


    for (size_t i = 0; content[i] != '\0'; i++)
    {
        if (content[i] == lineSeparator)
        {
            lines++;
        }
    }

    return lines;
}

size_t CountNotEmptyLines (char *content, char lineSeparator)
{
    assert(content);

    size_t lines = 0;

    bool text = content[0] != lineSeparator;

    for (size_t i = 0; content[i] != '\0'; i++)
    {
        if (content[i] == lineSeparator && text)
        {
            lines++;
        }

        text = content[i] != lineSeparator;
    }

    return lines;
}

int TextCtor (char *inputFileName, text_t *text)
{
    text->buffer = ReadFile (inputFileName, &text->bufferLen); // FIXME: also returns file size
    
    if (text->buffer == NULL) return COMMON_NULL_POINTER; 
    
    text->lines = MakePointers (text->buffer, '\n');

    if (text->lines == NULL) return COMMON_NULL_POINTER;

    return COMMON_OK;
}

int TextDtor (text_t *text)
{
    free (text->buffer);
    text->buffer = NULL;

    free (text->lines);
    text->lines = NULL;

    text->bufferLen = 0;

    return 0;
}