/*
Title: 14008175400_hw2.c
Author: Sarper Sakmak
ID: 14008175400
Section: 5
Assignment: CMPE Homework 2
Description:
        - This is Synchronization: Semaphores and critical section implementation assignment.
        - The program uses pthreads and a counting semaphore to limit the number of simultaneously active threads while processing all files in the given directory.
        - One thread is created per .txt file. Threads are created dynamically whenever an active thread terminates (main uses sem_wait before each pthread_create).
        - Deficient numbers are counted in each file and printed thread by thread.
        - Program works for any number of files.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/stat.h>

/* 
   STRUCT: ThreadArg
    
   Holds all data that must be passed to a worker thread.
   - thread_id  : the logical thread number (1-based) for printing
   - filename   : just the base filename (e.g. "file3.txt")
   - fullpath   : full relative path to the file (e.g. "myDir/file3.txt")
   - sem        : pointer to the global counting semaphore
   */
typedef struct {
    int         thread_id;
    char        filename[256];
    char        fullpath[512];
    sem_t      *sem;
} ThreadArg;

/*
   FUNCTION: is_deficient
   
   A positive integer n is called DEFICIENT if the sum of ALL its proper
   divisors (divisors strictly less than n, including 1) is LESS THAN n.

   Examples:
       n = 1  -> proper divisors: none       -> sum = 0  < 1  -> DEFICIENT
       n = 2  -> proper divisors: {1}        -> sum = 1  < 2  -> DEFICIENT
       n = 8  -> proper divisors: {1,2,4}    -> sum = 7  < 8  -> DEFICIENT
       n = 6  -> proper divisors: {1,2,3}    -> sum = 6 == 6  -> PERFECT (NOT deficient)
       n = 12 -> proper divisors: {1,2,3,4,6}-> sum = 16 > 12 -> ABUNDANT (NOT deficient)

   Parameters:
       n (int): the integer to test; must be >= 1.

   Returns:
       1  if n is deficient
       0  otherwise
   ----------------------------------------------------------------------- */
int is_deficient(int n)
{
    /* STEP 1: Handle the edge case where n <= 0; such numbers are not
               considered in the classical number-theory sense, so return 0. */
    if (n <= 0)
        return 0;

    /* STEP 2: Accumulate the sum of proper divisors.
               We start the sum at 0. By definition, 1 is always a proper
               divisor of any n >= 2, so we initialise sum = 1 and begin
               the divisor search from i = 2 to avoid counting 1 twice.
               For n == 1 the only candidate divisor would be 1 itself, but
               1 is NOT a proper divisor of 1 (proper means strictly less),
               so for n == 1 the sum stays 0. */
    int sum = (n > 1) ? 1 : 0;

    /* STEP 3: Iterate from i = 2 up to sqrt(n).
               For every i that divides n evenly, both i and n/i are divisors.
               Add both, but guard against adding n itself (when i == n/i == sqrt(n))
               and against counting the same divisor twice (when i == n/i). */
    for (int i = 2; (long long)i * i <= (long long)n; i++) {
        if (n % i == 0) {
            sum += i;                   /* i is a proper divisor          */
            if (i != n / i)
                sum += n / i;           /* n/i is a distinct proper divisor */
        }
    }

    /* STEP 4: A number is deficient when the divisor sum is strictly less
               than the number itself. Return 1 (true) or 0 (false). */
    return (sum < n) ? 1 : 0;
}

/*
   FUNCTION: thread_function
   
   Entry point for every worker thread.

   STEP 1: Cast the void* argument back to ThreadArg*.
   STEP 2: Open the file given in arg->fullpath for reading.
   STEP 3: Read integers one by one; for each integer call is_deficient()
           and increment the local counter when it returns 1.
   STEP 4: Close the file.
   STEP 5: Print the result in the required format.
   STEP 6: Release one slot on the semaphore (sem_post) so the main thread
           may create the next pending thread.
   STEP 7: Free the ThreadArg struct that was heap-allocated by main.
   STEP 8: Exit the thread (detached threads clean up automatically).
   ----------------------------------------------------------------------- */
void *thread_function(void *arg)
{
    /* STEP 1: Recover the argument struct. */
    ThreadArg *targ = (ThreadArg *)arg;

    /* STEP 2: Open the file. */
    FILE *fp = fopen(targ->fullpath, "r");
    if (fp == NULL) {
        perror("fopen");
        sem_post(targ->sem);   /* release slot even on error */
        free(targ);
        pthread_exit(NULL);
    }

    /* STEP 3: Count deficient numbers. */
    int num;
    long long deficient_count = 0;

    while (fscanf(fp, "%d", &num) == 1) {
        if (is_deficient(num))
            deficient_count++;
    }

    /* STEP 4: Close the file. */
    fclose(fp);

    /* STEP 5: Print result in the exact required format. */
    printf("Thread %d has found %lld deficient numbers in %s\n",
           targ->thread_id, deficient_count, targ->filename);
    fflush(stdout);

    /* STEP 6: Post the semaphore — one slot becomes available for main. */
    sem_post(targ->sem);

    /* STEP 7: Free the heap-allocated argument struct. */
    free(targ);

    /* STEP 8: Exit. Because the thread is detached, resources are freed
               automatically by the OS. */
    pthread_exit(NULL);
}

/*
   FUNCTION: main
   
   Orchestrates directory scanning, semaphore management, and thread creation.

   STEP 1:  Validate command-line arguments (exactly 2 required).
   STEP 2:  Parse threadNumber from argv[2].
   STEP 3:  Initialise the counting semaphore to threadNumber.
   STEP 4:  Open the directory given in argv[1] with opendir().
   STEP 5:  Iterate over directory entries with readdir(); for each .txt file:
                a) sem_wait() -- blocks if threadNumber threads are already active.
                b) Build a ThreadArg on the heap.
                c) Create a DETACHED pthread that runs thread_function().
   STEP 6:  After the loop, drain remaining semaphore slots (wait for the last
            batch of threads to finish) by calling sem_wait() threadNumber times.
   STEP 7:  Destroy the semaphore and close the directory.
   STEP 8:  Return EXIT_SUCCESS.
    */
int main(int argc, char *argv[])
{
    /* STEP 1: Validate arguments. */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directoryName> <threadNumber>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* STEP 2: Parse threadNumber. */
    int threadNumber = atoi(argv[2]);
    if (threadNumber <= 0) {
        fprintf(stderr, "Error: threadNumber must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    /* STEP 3: Initialise the counting semaphore.
               The semaphore is initialised to threadNumber so that up to
               threadNumber threads may be active simultaneously.
               (0 = process-local semaphore, not shared across processes) */
    sem_t sem;
    if (sem_init(&sem, 0, (unsigned int)threadNumber) != 0) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    /* STEP 4: Open the directory. */
    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("opendir");
        sem_destroy(&sem);
        return EXIT_FAILURE;
    }

    /* STEP 5: Scan directory entries and spawn threads. */
    struct dirent *entry;
    int thread_id = 0;          /* logical thread counter (1-based)  */

    while ((entry = readdir(dir)) != NULL) {

        /* STEP 5a: Skip entries that are not regular .txt files. */
        size_t len = strlen(entry->d_name);
        if (len < 4 || strcmp(entry->d_name + len - 4, ".txt") != 0)
            continue;

        /* STEP 5b: Build the full relative path (e.g. "myDir/file3.txt"). */
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", argv[1], entry->d_name);

        /* Optionally verify it is a regular file using stat. */
        struct stat st;
        if (stat(fullpath, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        /* STEP 5c: sem_wait() -- blocks until a thread slot is free.
                    This enforces the "at most threadNumber active threads" rule.
                    The main thread blocks here when all slots are taken, exactly
                    like a student waiting outside the instructor's office. */
        if (sem_wait(&sem) != 0) {
            perror("sem_wait");
            break;
        }

        /* STEP 5d: Allocate and populate the ThreadArg on the heap so that
                    it remains valid after this loop iteration advances. */
        thread_id++;
        ThreadArg *targ = (ThreadArg *)malloc(sizeof(ThreadArg));
        if (targ == NULL) {
            perror("malloc");
            sem_post(&sem);   /* release the slot we just consumed */
            break;
        }
        targ->thread_id = thread_id;
        targ->sem       = &sem;
        strncpy(targ->filename, entry->d_name, sizeof(targ->filename) - 1);
        targ->filename[sizeof(targ->filename) - 1] = '\0';
        strncpy(targ->fullpath, fullpath, sizeof(targ->fullpath) - 1);
        targ->fullpath[sizeof(targ->fullpath) - 1] = '\0';

        /* STEP 5e: Configure thread attributes to create a DETACHED thread.
                    Detached threads release their resources automatically
                    on termination; we do NOT join them. */
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&tid, &attr, thread_function, (void *)targ) != 0) {
            perror("pthread_create");
            free(targ);
            sem_post(&sem);   /* release the slot we just consumed */
            pthread_attr_destroy(&attr);
            break;
        }

        pthread_attr_destroy(&attr);
        /* The thread is now running; main loops back to readdir() immediately. */
    }

    /* STEP 6: Wait for the final batch of threads to complete.
               Since threads are detached and we cannot join them, we drain
               the semaphore by calling sem_wait() exactly threadNumber times.
               Each still-running thread will sem_post() once upon completion,
               so when we have acquired all threadNumber slots the last thread
               has finished. */
    for (int i = 0; i < threadNumber; i++) {
        if (sem_wait(&sem) != 0) {
            perror("sem_wait (drain)");
            break;
        }
    }

    /* STEP 7: Release OS resources. */
    closedir(dir);
    sem_destroy(&sem);

    /* STEP 8: Done. */
    return EXIT_SUCCESS;
}