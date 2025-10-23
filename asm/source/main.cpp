#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assembler.h"
#include "debug.h"
#include "file.h"

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf ("Run program like this: %s file.my_asm\n", argv[0]);

        return 1;
    }

    asm_t myAsm = {};

    AsmCtor (&myAsm /*, inputBuffer*/);

    int status = AsmRead (&myAsm, argv[1]);
    if (status != 0)
    {
        ERROR ("%s", "Error in AsmRead()");

        return status;
    }

    status = Assemble (&myAsm);
    if (status != 0)
    {
        ERROR ("%s", "Error in assembling")
        AsmDtor (&myAsm);
        
        return status;
    }

    status = PrintBinary ("../spu/out.spu", &myAsm); // TODO: add outputfile argument
    if (status != 0)
    {
        AsmDtor (&myAsm);
        ERROR ("status = %d;", status)
        
        return status;
    }

    AsmDtor (&myAsm);

    return 0;
}