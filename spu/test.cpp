#include <stdio.h>

#define MACRO(a) #a

#define MACRO_X(a) MACRO(a)

int main()
{
    printf ("%s", MACRO_X(__LINE__));
}