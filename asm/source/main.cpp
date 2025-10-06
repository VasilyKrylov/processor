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
    
    // size_t numberOfOperands = 0;

    asm_t myAsm = {};
    int status = AsmCtorAndRead (argv[1], &myAsm);
    DEBUG_LOG ("buffer: \"%s\"\n", myAsm.text.buffer);
    if (status != 0)
    {
        ERROR ("%s", "Error in AsmCtorAndRead()");
    }


    status = Assemble (&myAsm);
    if (status != 0)
    {
        ERROR ("%s", "Error while assembling")
        AsmDtor (&myAsm);
        // function return 0 or 1
        // 0 is ok
        // 1 is error
        // when to print error message
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