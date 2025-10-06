#ifndef K_FILE_H
#define K_FILE_H

#include <sys/types.h>

struct line 
{
    char *start = NULL;
    size_t len = 0;
};

struct text_t
{
    char *buffer = NULL;
    size_t bufferLen = 0;

    line *lines = NULL;
};

#ifdef PRINT_DEBUG // TODO: while(0)
    void PrintContent (char *content);
    void PrintLinesArray (line *linesArray);

    #define PRINT_CONTENT(content) PrintContent (content);
    #define PRINT_LINES_ARRAY(linesArray) PrintLinesArray (linesArray);
#else
    #define PRINT_CONTENT(content) 
    #define PRINT_LINES_ARRAY(linesArray) 
#endif // PRINT_DEBUG

char *ReadFile (char *inputFileName, size_t *bufferLen);
line *MakePointers (char *buffer, char lineSeparator);
ssize_t GetFileSize (char *fileName);

size_t CountLines (char *content, char lineSeparator);
size_t CountNotEmptyLines (char *content, char lineSeparator);

int TextCtor (char *inputFileName, text_t *text);
int TextDtor (text_t *text);

#endif // K_FILE_H