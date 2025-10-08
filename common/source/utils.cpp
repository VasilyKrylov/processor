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