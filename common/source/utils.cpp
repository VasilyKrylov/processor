#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "utils.h"

void Swap (size_t *a, size_t *b)
{
    assert (a);
    assert (b);

    size_t c = *b;
               *b = *a;
                    *a = c;
}

// counts how many chars in elements presented in s
size_t CountChr (char *s, const char element)
{
    assert(s);

    size_t counter = 0;

    for (size_t i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == element)
        {
            counter++;
        }
    }

    return counter;
}

void StrReplace (char *s, const char *oldValues, const char newValue)
{
    assert(s);
    assert(oldValues);

    for (size_t i = 0; s[i] != '\0'; i++)
    {
        for (size_t j = 0; oldValues[j] != '\0'; j++)
        {
            if (s[i] == oldValues[j])
            {
                s[i] = newValue;

                break;
            }
        }
    }
}

size_t GetWordLen (char *s, const char *delimiters)
{
    // FIXME: remove this function and make constant delimetersWord
    assert (s); 
    assert (delimiters); 

    return strcspn (s, delimiters);
}

char *SkipSpaces (char *s)
{
    assert (s);

    s += strspn (s, " ");

    return s; 
}

int PrintSymbols (FILE *outputFile, size_t cnt, char c)
{
    // NOTE: I know better to print 4 or 8 symbols at once, but I have no time for this know

    for (size_t i = 0; i < cnt; i++)
    {
        int status = fprintf (outputFile, "%c", c);
        if (status < 0)
            return COMMON_ERROR_WRITE_TO_FILE;
    }

    return COMMON_OK;
}

unsigned long HashDjb2 (char *str) 
{
    unsigned long hash = 5381;
    int c = '\1';

    DEBUG_LOG("*str = \"%s\";", str);

    
    // for (size_t i = 0; i < len; i++)
    // {
    //     hash = ((hash << 5) + hash) + (unsigned long)str[i];
        
    //     DEBUG_LOG ("%c\t%d", c, c);
    // }

    while (c != '\0')
    {
        hash = ((hash << 5) + hash) + (unsigned long)c;
        c = *(str++);
    }

    return hash;
}