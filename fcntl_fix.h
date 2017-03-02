#ifndef _FCNTL_FIX_H_
#define _FCNTL_FIX_H_

#include "help.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


    // structure for arguments of function passed to threads
struct fixedThreadArg {
    unsigned int threadid;
    const char* infilename;
    const char* outfilename;
    FILE* logfp;
    FILE* infilefp;
};

struct flock fixedlock;             // the fcntl lock structure
 
void *fixedReader(void *arg);    // reader threads function
void *fixedWriter(void *arg);    // writer threads function


    // The default race condition implementation function, create m+n threads with m writers and n readers
    // have them race for access to infile, write lock details to outfile
void fcntl_fixed(const char* infile, const char* outfile)
{
    int i = 0;                      // iterator
    int NUMBER = get_random(200);   // number of threads
    
    memset(&fixedlock, 0, sizeof(fixedlock)); // initialize struct at this location in memory    
    
    struct fixedThreadArg Arguments[NUMBER];    // making an array for argument variables to avoid double free error  
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
    { printf("\nError opening output file: \"%s\" from fcntl_fix()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\n\nfcntl_fix() test run started on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
            
    printf("\n\nCreating %d threads in fcntl_fix() function.\n", NUMBER);
    fprintf(Arguments[0].logfp, "Total Threads: %d\n", NUMBER);
    fclose(Arguments[0].logfp);
    
    
    pthread_t RW_threads[NUMBER];  // to hold a random number of threads
      
        // now randomly distribute threads into readers and writers 
    for(i = 0, readCount = 0; i < NUMBER; i++)
    {
        Arguments[i].threadid = i;
        
        if(get_random(2) == 1)
        {
           pthread_create(&RW_threads[i], NULL, fixedReader, &Arguments[i]);
           
        }
        else { 
            pthread_create(&RW_threads[i], NULL, fixedWriter, &Arguments[i]);
            
        }
    
    }
    
      // fcntl_fixed() waits for all threads to end
    for(i = 0; i < NUMBER; i++)
        if(pthread_join(RW_threads[i], NULL) != 0) printf("\nSome thread is stuck ._.\n");


        // add ending information for this entry to outfile
    Arguments[0].logfp = fopen(outfile, "a");
    if(!Arguments[0].logfp)
    { printf("\nError opening output file: \"%s\" from fcntl_fix()\n", outfile); return; }
    
    fprintf(Arguments[0].logfp, "\nfcntl_fix() test run end on "); writeTimeTo(Arguments[0].logfp);
    fprintf(Arguments[0].logfp, "\n");             
    fclose(Arguments[0].logfp);


}// end fcntl_fix()



    // fixedWriter function that will be called by writer threads, edits infile, and posts timestamps to outfile
void * fixedWriter(void *arg)
{
    
    struct fixedThreadArg *Arg = arg;    // arguments 
    
    // Output that a writer was created.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Writer thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    // Initialize Lock struct, and obtain a lock to the file.
    int fd;          
    while(readCount);       // wait on readers
    while(fixedlock.l_pid); // wait on other writers
    
    fd = open(Arg->infilename, O_WRONLY);   // open infile in write only mode
    fixedlock.l_type = F_WRLCK;             // lock type is writelock (exclusive)
    fixedlock.l_pid = pthread_self();       // make this lock your own
    fcntl(fd, F_SETLKW, &fixedlock);        // wait to acquire lock
        
    // Output that this writer now holds the lock.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Writer thread %d) threadID: %u\tAcquired Exclusive lock at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
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
    
    // Output that this writer will now release the exclusive lock.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Writer thread %d) threadID: %u\tReleases Exclusive lock at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);

    // Release the lock.
    fixedlock.l_type = F_UNLCK;         // change lock type to unlock
    fixedlock.l_pid  = 0;               // throw away the lock 
    fcntl (fd, F_SETLKW, &fixedlock);   // release the shared lock
    close (fd);                    // close file descriptor
    
    
}// end fixedWriter()



    // fixedReader function to read from the input file, and posts time stamps to the output file.
void *fixedReader(void *arg)
{ 
    struct fixedThreadArg *Arg = arg;    // arguments 
    
    // Output that a reader was created
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Reader thread %d) threadID: %u\tCreated at: ", Arg->threadid, (unsigned int)pthread_self());
    writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    readCount++;
    // Initialize Lock struct, and obtain a lock to the file.
    int fd;          
    fd = open(Arg->infilename, O_RDONLY);   // open infile in Readonly mode
    fixedlock.l_type = F_RDLCK;             // lock type is Readlock (shared)
    fixedlock.l_pid = pthread_self();       // make this lock your own
    fcntl(fd, F_SETLKW, &fixedlock);        // wait to acquire lock

    // Output that this reader now holds the shared lock.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Reader thread %d) threadID: %u\tAcquires Shared lock at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    // Output that this reader will now read
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tBegins reading \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    
    
        // READING (Critical Region) (We enter here after aquiring WRLCK)
    Arg->infilefp = fopen(Arg->infilename, "r"); // we will read the input file in this function
    unsigned int n; while(fscanf(Arg->infilefp, "%u", &n) != EOF);
    fclose(Arg->infilefp);    
        // Done reading.
    
    
    // Output that this reader is done reading
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp, "(Reader thread %d) threadID: %u\tEnds   reading \"%s\" at: ", Arg->threadid,
    (unsigned int)pthread_self(), Arg->infilename); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);        
    
    // Output that this reader will now release its hold on the shared lock.
    Arg->logfp = fopen(Arg->outfilename, "a");
    fprintf(Arg->logfp,"(Reader thread %d) threadID: %u\tReleases Shared lock at: ", Arg->threadid,
    (unsigned int)pthread_self()); writeTimeTo(Arg->logfp); fprintf(Arg->logfp, "\n");
    fclose(Arg->logfp);
    readCount--;
    
    if(readCount == 0)
    {   // Release the lock.
        fixedlock.l_type = F_UNLCK;         // change lock type to unlock
        fixedlock.l_pid = 0;                // let the writers in
        fcntl (fd, F_SETLKW, &fixedlock);   // release the shared lock
        close (fd);                         // close file descriptor
    }
    else close(fd);
    
    
}// end fixedReader()

#endif // _FCNTL_FIX_H_
