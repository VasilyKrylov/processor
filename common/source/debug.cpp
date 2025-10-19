#include <stdio.h>

#include "debug.h"

void StackPrintError (int error)
{
    DEBUG_LOG ("error = %d\n", error);

    if (error == COMMON_OK) return;

    printf("%s", RED_BOLD_COLOR);

    if (error & COMMON_ERROR_ALLOCATING_MEMORY)     printf("%s", "Error allocating memory\n");
    if (error & COMMON_ERROR_REALLOCATING_MEMORY)   printf("%s", "Error reallocating memory\n");
    if (error & COMMON_ERROR_OPENING_FILE)          printf("%s", "Error opening file\n");
    if (error & COMMON_ERROR_NULL_POINTER)                printf("%s", "Null pointer on thing, that should be not NULL\n");
    if (error & COMMON_ERROR_SSCANF)                printf("%s", "Can't read all arguments given to sscanf()\n");

    printf("%s", COLOR_END);
}