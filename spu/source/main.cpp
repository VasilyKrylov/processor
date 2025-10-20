#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spu.h"
#include "debug.h"
#include "stack.h"
#include "utils.h"

// TODO: USE VERIFICATOR!!!!!!!!!!
// TODO: macro for if error return else nothing
// TODO: массив указателей
// TODO: генерация spu_commands.cpp
int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf ("Run program like this: %s file.spu\n", argv[0]);
        
        return 0;
    }
    
    spu_t spu = {};
    SPU_CTOR (&spu);
    int result = SpuRead (&spu, argv[1]);

    if (result != 0)
    {
        SpuDtor (&spu);

        return result;
    }
    // TODO: SpuVerify()

    result = SpuRun (&spu);
    if (result != 0)
    {
        SpuDtor (&spu);

        return result;
    }

    SpuDtor (&spu);

    return 0;
}
