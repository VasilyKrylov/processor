#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assembler.h"
#include "debug.h"
#include "file.h"

// TODO: new macro
/*
macro for :
    int status = AsmRead (&myAsm, argv[1]);

    if (status != 0)
    {
        ERROR ("%s", "Error in AsmRead()");

        return status;
    }
*/

// TODO: print bytecode size
// TODO: код генерация функций сравнения
// TODO: код генерация свитч кейса

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf ("Run program like this: %s file.my_asm\n", argv[0]);

        return 7;
    }

    asm_t myAsm = {};

    AsmCtor (&myAsm /*, inputBuffer*/);

    int status = AsmRead (&myAsm, argv[1]);
    if (status != 0)
    {
        ERROR ("%s", "Error in AsmRead()");

        return status;
    }

    // TODO: make argument with argument number of pass
    // TODO: Macros WRITE_TO_BYTECODE
    /* 
    1 проход:
        не аллоцируем массив байткода, просто считаем myAsm-> ip
        заполняем массив меток
    
    2 проход:
        аллоцируем массив байткода на посчитанное значение из первого прохода
        проверяем, что все метки, на которые выполняется прыжок не 0
    */
    /*
    функция токенизации:
        на вход char *

        на выход массив токенов
    */
    status  = Assemble (&myAsm, 1);
    if (status != 0)
    {
        ERROR ("%s", "Error while assembling")
        AsmDtor (&myAsm);
        
        return status;
    }

    status  = Assemble (&myAsm, 2);
    if (status != 0)
    {
        ERROR ("%s", "Error while assembling")
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