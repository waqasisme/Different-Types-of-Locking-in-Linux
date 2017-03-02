#ifndef _NORMAL_H_
#define _NORMAL_H_

#include "help.h"
#include <pthread.h>

    // structure for arguments of function passed to threads
struct nThreadArg {
    unsigned int threadid;
    const char* infilename;
    const char* outfilename;
    FILE* logfp;
    FILE* infilefp;
};


void *nReader(void *arg);    // reader threads function
void *nWriter(void *arg);    // writer threads function


    // The default race condition implementation function, create m+n threads with m writers and n readers
    // have them race for access to infile, write lock details to outfile
void normal(const char* infile, const char* outfile)
{
    
    int i = 0;                     // iterator
    int NUMBER = get_random(100);   // number of threads
    
    struct nThreadArg Arguments[NUMBER];    // making an array for argument variables to avoid double free error  
        // initialize argument variables
    for(i = 0; i < NUMBER; i++)
    {
        Arguments[i].threadid = 0;
        Arguments[i].infilename = infile;
        Arguments[i].outfilename = outfile;
        Arguments[i].logfp = NULL;
        Arguments[i].infilefp = NULL;
    }
    
        // add info for this run to outfile
    Arguments[0].logfp = fopen(outfile, "a");
    if(!Arguments[0].logfp)
    { printf("\nError opening output file: \"%s\" from Normal()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\n\nNormal() test run started on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
            
    printf("\n\nCreating %d threads in Normal() function.\n", NUMBER);
    fprintf(Arguments[0].logfp, "Total Threads: %d\n", NUMBER);
    fclose(Arguments[0].logfp);
    
    
    pthread_t RW_threads[NUMBER];  // to hold a random number of threads
      
        // now randomly distribute threads into readers and writers 
    int dist;
    for(i = 0; i < NUMBER; i++)
    {
        Arguments[i].threadid = i;
        dist = get_random(4);

        if(dist == 2 || dist == 1 || dist == 3)
        {
           pthread_create(&RW_threads[i], NULL, nReader, &Arguments[i]);
        }
        else { 
            pthread_create(&RW_threads[i], NULL, nWriter, &Arguments[i]);
  
        }
    
    }
    
      // normal() waits for all threads to end
    for(i = 0; i < NUMBER; i++)
        if(pthread_join(RW_threads[i], NULL) != 0) printf("\nSome thread is stuck ._.\n");


        // add ending information for this entry to outfile
    Arguments[0].logfp = fopen(outfile, "a");
    if(!Arguments[0].logfp)
    { printf("\nError opening output file: \"%s\" from Normal()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\nNormal() test run end on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
    fclose(Arguments[0].logfp);


}// end normal()



    // nWriter functin that will be called by writer threads, edits infile, and posts timestamps to outfile
void * nWriter(void *arg)
{
    struct nThreadArg *Arg = arg;    // arguments 
    
    // Output that a writer was created.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Writer thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    // Output that this writer will now write.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Writer thread %d) threadID: %u\tBegins writing \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
   
        // WRITING (critical region) (that we enter here without locks)
    Arg->infilefp = fopen(Arg->infilename, "a"); // we will write to the input file in this function
    int i = 0;
    for(i = 0; i < 5000; i++)
        fprintf(Arg->infilefp, "%d\n", i);
    fclose(Arg->infilefp);
        // Done writing
    
    // Output that this writer is done writing
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Writer thread %d) threadID: %u\tEnds   writing \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    
}// end nWriter()



    // Reader function to read from the input file, and posts time stamps to the output file.
void *nReader(void *arg)
{ 
    struct nThreadArg *Arg = arg;    // arguments 
    
    // Output that a reader was created
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Reader thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    // Output that this reader will now read
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tBegins reading \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    
        // READING (Critical Region) (that we enter here without locks)
    Arg->infilefp = fopen(Arg->infilename, "r"); // we will read the input file in this function
    unsigned int n; while(fscanf(Arg->infilefp, "%u", &n) != EOF);
    fclose(Arg->infilefp);    
        // Done reading.
    
    
    // Output that this reader is done reading
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tEnds   reading \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);        
    
    
}// end nReader()


#endif // _NORMAL_H_
