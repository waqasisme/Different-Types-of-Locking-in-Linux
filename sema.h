#ifndef _SEMA_H_
#define _SEMA_H_

#include <semaphore.h>
#include "help.h"


sem_t mutex;        // mutex struct
sem_t rw_lock;      // read-write lock struct 


    // structure for arguments of function passed to threads
struct ThreadArg {
    unsigned int threadid;
    const char* infilename;
    const char* outfilename;
    FILE* logfp;
    FILE* infilefp;
};


void *sReader(void *arg);    // reader threads function
void *sWriter(void *arg);    // writer threads function


    // The main semaphore implementation function, create m+n threads with m writers and n readers
    // have them race for access to infile, write lock details to outfile
void Sem(const char* infile, const char* outfile)
{
    
    int i = 0;                     // iterator
    int NUMBER = get_random(200);  // number of threads
    sem_init(&mutex, 0, 1);        // initialize mutex lock (for writers exclusivity)
    sem_init(&rw_lock, 0, 1);      // initialize readers writers lock (for shared readers)


    struct ThreadArg Arguments[NUMBER];    // making an array for argument variables to avoid double free error  
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
    { printf("\nError opening output file: \"%s\" from Sema()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\n\nSema() test run started on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
            
    printf("\n\nCreating %d threads in Sema() function.\n", NUMBER);
    fprintf(Arguments[0].logfp, "Total Threads: %d\n", NUMBER);
    fclose(Arguments[0].logfp);
    
    pthread_t RW_threads[NUMBER]; // to hold a random number of threads
    
        // now randomly distribute threads into readers and writers 
    for(i = 0, readCount = 0; i < NUMBER; i++)
    {
        Arguments[i].threadid = i;
        
        if(get_random(2) == 1)
        {
           pthread_create(&RW_threads[i], NULL, sReader, &Arguments[i]);
           
        }
        else { 
            pthread_create(&RW_threads[i], NULL, sWriter, &Arguments[i]);
            
        }
    
    }

     for(i = 0; i < NUMBER; i++)
        if(pthread_join(RW_threads[i], NULL) != 0) printf("\nSome thread is stuck ._.\n");

    
    sem_destroy(&mutex);
    sem_destroy(&rw_lock); 

        // add ending information for this entry to outfile
    Arguments[0].logfp = fopen(outfile, "a");
    if(!Arguments[0].logfp)
    { printf("\nError opening output file: \"%s\" from Sema()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\nSema() test run end on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
    fclose(Arguments[0].logfp);
    
}// end Sem()


    // sWriter function to edit the input file, and posts time stamps to the output file
void * sWriter(void *arg)
{
    
    struct ThreadArg *Arg = arg;    // arguments 

    // Output that a writer was created.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Writer thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    sem_wait(&rw_lock); // waits for readers to end
    
    // Output that this writer will now write.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Writer thread %d) threadID: %u\tBegins writing \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
        // WRITING (critical region)
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


    sem_post(&rw_lock); // gives lock back
    
}

    // sReader function to read from the input file, and posts time stamps to the output file
void *sReader(void *arg)
{ 
    struct ThreadArg *Arg = arg;    // arguments 
    
    // Output that a reader was created
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Reader thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    // Acquire mutex lock to block writers
    sem_wait(&mutex);
    readCount++;    
    // Acquire shared RW_Lock
    if(readCount == 1)
    {
        sem_wait(&rw_lock);    
    }
    
    // Output that there is at least one reader in queue
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tShared RW_Lock Aquired at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    sem_post(&mutex);   // end of atomic instructions
    
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


    sem_wait(&mutex);  // get lock to decrement readCount  
    readCount--;
    
    // Output that this reader is done reading
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tEnds   reading \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);        
    
    // Output that this reader has released its hold on the shared lock
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %d\t Shared RW_Lock Released at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
        
    // no more readers, release shared lock
    if(readCount == 0)
    {   
        sem_post(&rw_lock); 
    }
    
    // let the writers in
    sem_post(&mutex);
    
}


#endif // _SEMA_H_
