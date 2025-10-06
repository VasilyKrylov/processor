#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spu.h"
#include "debug.h"
#include "stack.h"
#include "utils.h"

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf ("Run program like this: %s file.spu\n", argv[0]);
        
        return 0;
    }
    
    spu_t processor = {}; // NOTE: but why I should write = {};
                          // if there is NO default values in spu_t
                          // and I call ProcessorCtor on the next line
    SpuCtor (&processor, argv[1]);

    // TODO: SpuVerify()

    int result = SpuRun (&processor);
    if (result != 0)
    {
        SpuDtor (&processor);

        return -1;
    }

    SpuDtor (&processor);

    return 0;
}
