#ifndef _MONITORS_H_
#define _MONITORS_H_

#include<semaphore.h>

struct Monitor {

    int read_count;
    sem_t mutex;
    sem_t rw_lock;
    
    void (*initialise)();     
    void (*end)();
    void (*acquire)();
    void (*release)();

};

struct mThreadArg {
    unsigned int threadid;
    const char* infilename;
    const char* outfilename;
    FILE* logfp;
    FILE* infilefp;
    int bit;
};


void *mReader(void * arg);   // reader lock function
void *mWriter(void * arg);   // reader threads function
void acquire();              // monitors acquire function
void release();              // monitors release function


struct Monitor M;            // Our Monitor object


    // Destroy Locks when done
void end()
{
    sem_destroy(&(M.mutex));
    sem_destroy(&(M.rw_lock)); 
}

   // Initialize Monitor object variables
void initialise()
{
    M.acquire = &acquire;   // load acquire and release 
    M.release = &release;   // functions into pointers
    
    sem_init(&M.mutex, 0, 1);       // initialize mutex lock (for writers exclusivity)
    sem_init(&M.rw_lock, 0, 1);     // initialize readers writers lock (for shared readers)
    M.read_count = 0;               // initialize read count as 0

}


    // The main monitor implementation function, create m+n threads with m writers and n readers
    // have them race for access to infile, write lock details to outfile
void monitors(const char* infile, const char* outfile)
{
    // initialize monitor struct    
    M.initialise = &initialise;
    M.end = &end;
    M.initialise(); 
    
    
    int i = 0;                     // iterator
    int NUMBER = get_random(500);  // number of threads
    

    struct mThreadArg Arguments[NUMBER];    // making an array for argument variables to avoid double free error  
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
    { printf("\nError opening output file: \"%s\" from Monitor()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\n\nMonitor() test run started on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
            
    printf("\n\nCreating %d threads in Monitor() function.\n", NUMBER);
    fprintf(Arguments[0].logfp, "Total Threads: %d\n", NUMBER);
    fclose(Arguments[0].logfp);
    
    pthread_t RW_threads[NUMBER]; // to hold a random number of threads
    
        // now randomly distribute threads into readers and writers 
    for(i = 0; i < NUMBER; i++)
    {
        Arguments[i].threadid = i;

        if(get_random(2) == 1)
        {
           pthread_create(&RW_threads[i], NULL, mReader, &Arguments[i]);
          
        }
        else { 
            pthread_create(&RW_threads[i], NULL, mWriter, &Arguments[i]);
            
        }
    
    }

     for(i = 0; i < NUMBER; i++)
        if(pthread_join(RW_threads[i], NULL) != 0) printf("\nSome thread is stuck ._.\n");


    // destroy locks
    M.end(&M);

    
        // add ending information for this entry to outfile
    Arguments[0].logfp = fopen(outfile, "a");
    if(!Arguments[0].logfp)
    { printf("\nError opening output file: \"%s\" from Monitors()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\nMonitors() test run end on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
    fclose(Arguments[0].logfp);
    
}


    // mWriter function to edit the input file, and posts time stamps to the output file
void * mWriter(void *arg)
{
    
    struct mThreadArg *Arg = arg;    // arguments 

    // Output that a writer was created.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Writer thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);Arg->bit = 0;
    
    
    // try to aquire rw_lock
    M.acquire(Arg);
    
    
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

    // release lock when done
    M.release(Arg);
    
       
}

    // mReader function to read from the input file, and posts time stamps to the output file
void *mReader(void *arg)
{ 
    struct mThreadArg *Arg = arg;    // arguments 
    
    // Output that a reader was created
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Reader thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);Arg->bit = 1;
    
    
    // Wait to obtain lock
    M.acquire(Arg);
        
        
    // Output that there is at least one reader in queue
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tShared RW_Lock Aquired at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
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
    
    // Output that this reader has released its hold on the shared lock
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %d\t Shared RW_Lock Released at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    // Release the lock when done
    M.release(Arg);
    
}


void acquire(struct mThreadArg* Arg)
{
    if(Arg->bit != 0)   // if reader
    {   // exclusive access to read_count variable
        sem_wait(&M.mutex);
        M.read_count++;    
    
        if(M.read_count == 1)   // while there is at least one reader
            sem_wait(&M.rw_lock);    // obtain rw_lock
        
        sem_post(&M.mutex); // end exclusive access
    }
    else sem_wait(&M.rw_lock);  // if writer, wait to obtain rw_lock
    
}

    // releases locks when done
void release(struct mThreadArg* Arg)
{
    if(Arg->bit != 0)   // if reader
    {   // exclusive access to readCount variable
        sem_wait(&M.mutex);
        M.read_count--;
    
        if(M.read_count == 0)   // if no more readers, give rw_lock away
            sem_post(&M.rw_lock);
    
        sem_post(&M.mutex); // end exclusive access
    
    }
    else sem_post(&M.rw_lock);  // if writer, simply release lock
    
}

#endif // _MONITORS_H_
