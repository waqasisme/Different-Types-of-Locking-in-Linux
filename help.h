#ifndef _HELP_H_
#define _HELP_H_

#include <sys/time.h>   /* for timeval        */
#include <time.h>       /* for tm struct      */       
#include <stdlib.h>     /* rand() and srand() */
#include <stdio.h>      /* printf...          */


int readCount = 0;  // number of readers, to implement readers preferred solution.

    /* Generates a random number, if called first time, seeds with time() to initialize PRNG. */
static int seeded = 0;
int get_random(int limit)
{
    // seed if being called for the first time
    if(!seeded) { srand(time(NULL)); seeded = 1;}
    
    // return a random number in the range [1..limit]    
    return (rand() % limit + 1);    // return this random number, will always be in range [1..limit]
}


    /* Prints the current system time down to milliseconds. */
void print_time()
{
        // get current system time
    struct timeval tv;
    struct tm* ptm;
    gettimeofday(&tv, NULL); ptm = localtime(&tv.tv_sec);
    
    long milliseconds = tv.tv_usec / 1000;  // compute milliseconds from microseconds
    char time_string[40];

        // build time string in following format
    strftime(time_string, sizeof (time_string), "%d/%m/%y %H.%M.%S", ptm);  
    
        // print time string
    printf("%s.%03ld", time_string, milliseconds);
}


    /* Prints time stamp to file. */
void writeTimeTo(FILE * fp)
{
        // get current system time
    struct timeval tv;
    struct tm* ptm;
    gettimeofday(&tv, NULL); ptm = localtime(&tv.tv_sec);
    
    long milliseconds = tv.tv_usec / 1000;  // compute milliseconds from microseconds
    char time_string[40];

        // build time string in following format
    strftime(time_string, sizeof (time_string), "%d/%m/%y %I.%M.%S", ptm);  
    
        // write string to file
    fprintf(fp, "%s.%03ld", time_string, milliseconds);
}


    /* Creates a new text file and fills it with numbers. */
void generateTestFile(const char* filename)
{
    // open file
    FILE* fp = fopen(filename, "w");
 
    if(!fp) {  printf("\nError Generating Test File.\n");  return; }

    printf("\nCreating Test File with name \"%s\" and filling it with 50 Million +ve numbers.\n", filename);
        // fill this file with numbers
    unsigned int smallLAST = 50000;  // Fifty Thousand
    unsigned int bigLAST = 2500000;  // Twenty five Million
    
    unsigned int i;
    //for(i = 0; i < smallLAST; i++)
    //    fprintf(fp, "%u\n", i);
    
    
    for(i = 0; i < bigLAST; i++)
        fprintf(fp, "%u\n", i);
    
    
    fclose(fp); // close the test file
    printf("Done. \"%s\" Generated.\n", filename);
}



#endif // _HELP_H_


