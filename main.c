#include <stdio.h>
#include "help.h"
#include "normal.h"
#include "fcntl_default.h"
#include "fcntl_fix.h"
#include "sema.h"
#include "monitors.h"


int main(int argc, char* argv[])
{
    printf("\n Program \"%s\" started at: ", (argv[0] + 2)); print_time();
    
    char* infiles[] = {
            "normal.in", 
            "fcntl.in", "fcntl_fix.in",
            "semaphores.in",
            "monitors.in"
                     };
                     
    char* outfiles[] = {
            "normal.out",
            "fcntl.out", "fcntl_fix.out",
            "semaphores.out",
            "monitors.out"
                    };
    
    int i;
    for(i = 0; i < 5; i++)
        generateTestFile(infiles[i]);
    
    normal(infiles[0], outfiles[0]);
    fcntl_advisory(infiles[1], outfiles[1]);
    fcntl_fixed(infiles[2], outfiles[2]);    
    Sem(infiles[3], outfiles[3]);
    monitors(infiles[4], outfiles[4]);
    
    printf("\nProgram \"%s\" ended at: ", (argv[0] + 2)); print_time();
    printf("\n");
    
    return 0;

}
