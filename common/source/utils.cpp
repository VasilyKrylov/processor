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
size_t CountChrs (char *s, const char *elements)
{
    size_t counter = 0;
    for (size_t i = 0; s[i] != '\0'; i++)
    {
        for (size_t j = 0; elements[j] != '\0'; j++)
        {
            if (s[i] == elements[j])
            {
                counter++;
                break;
            }
        }
    }

    return counter;
}

// TODO: replaceChar
// size_t ReplaceChar ()