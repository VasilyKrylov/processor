#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assembler.h"
#include "debug.h"
#include "file.h"

// TODO: print bytecode size
// TODO: print version
int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf ("Run program like this: %s file.my_asm\n", argv[0]);

        return 0;
    }

    asm_t myAsm = {};

    // TODO: CtorAndRead()
    int status = AsmRead (&myAsm, argv[1]);

    if (status != 0)
    {
        ERROR ("%s", "Error in AsmRead()");
    }


    status = Assemble (&myAsm);
    if (status != 0)
    {
        ERROR ("%s", "Error while assembling")
        AsmDtor (&myAsm);
        
        return status;
    }

    status = PrintBytecode ("out.spu", &myAsm); // TODO: add outputfile argument
    if (status != 0)
    {
        AsmDtor (&myAsm);
        ERROR ("status = %d;", status)
        
        return status;
    }

    // free (buffer);
    // free (bytecode);
    // buffer   = NULL;
    // bytecode = NULL;

    AsmDtor (&myAsm);
    return 0;
}