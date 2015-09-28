/* 
 * Operating Systems  (2INC0)   Practical Assignment
 * Threaded Application
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * ”Extra” steps can lead to higher marks because we want students to take the initiative. 
 * Extra steps can be, for example, in the form of measurements added to your code, a formal 
 * analysis of deadlock freeness etc.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>     // for usleep()
#include <time.h>       // for time()
#include <pthread.h>
#include <semaphore.h>

#include "prime.h"

typedef struct {
    int base;
    sem_t* sem;
} Data;

typedef enum {false, true} bool;

static void rsleep (int t);

static pthread_mutex_t* getLock(int blockNr)
{
    static pthread_mutex_t locks[(NROF_SIEVE / 64) + 1];
    static bool initialized = false;
    // RIP in boolean
    if (initialized == false)
    {
        int  i;
        for (i = 0; i < locks / sizeof(pthread_mutex_t); i++)
        {
            locks[i] = PTHREAD_MUTEX_INITIALIZER;
        }
        initialized = true;
    }
    // Return the address to lock for block blockNr
    return &locks[blockNr];
}

// Obtain the lock for a specific block
static void lock(int blockNr)
{
    pthread_mutex_lock(getLock(blockNr));
}

// Release the lock for a specific block
static void unlock(int blockNr)
{
    pthread_mutex_unlock(getLock(blockNr));
}

// Remove all bits from blockNr which are set in val. Effectively calls
// oldValue ^= val. The locks ensure only one thread accesses a specific block at a time.
//
// Returns whether the value of the block changed
static int scrap(int blockNr, int val)
{
    lock(blockNr);
    unsigned long long old = buffer[blockNr];
    buffer[blockNr] = buffer[blockNr] & ~val;
    int changed = buffer[blockNr] != old;
    unlock(blockNr);
    return changed;
}

// Entry point for threads. Scraps all the numbers which are a multiple of the base.
static void run_sieve(void* args)
{
    Data* data = args;
    int val = 0;
    int blockNr = 0;
    int base = data->base;
    for (n = base * 2; n < MAX_SIEVE; n = n + base)
    {
        // n is twice the base, so it cannot be 0 (as base is guaranteed to be >= 1)
        if (n % 64 == 0)
        {
            // If scrap did not change anything, it's very unlikely it will find results further on
            if (scrap(val, blockNr))
            {
                break;
            }
            val = 0;
            blockNr = blockNr + 1;
        }
        int bitpos = n - (blockNr * 64);
        val = val | (1 << bitpos);
    }
    sem_post(data->sem);
}

int main (void)
{
    // TODO: start threads generate all primes between 2 and NROF_SIEVE and output the results
    // (see thread_malloc_free_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)

    // Initialize block locks and semaphore
    getLock(0);
    sem_t sem;
    sem_init(&sem, 0, NROF_THREAD);

    // For each number within 1 and MAX_SIEVE
    for (i = 2; i <= MAX_SIEVE; i++)
    {
        // Wait for the semaphore to ensure we don't use more than NROF_THREAD
        sem_wait(&sem);
        Data data;
        data.base = i;
        data.sem = &sem;
        pthread_t thread;
        // Start the thread with the given data
        pthread_create(&thread, NULL, &run_sieve, &data);
    }

    // Wait until all semaphore locks are for the main thread
    // As we don't want to store NROF_THREAD pthread_t structs,
    // I went with this (cheaper) version
    int n = NROF_THREAD;
    while (n--)
    {
        sem_wait(&sem);
    }
    sem_destroy(&sem);

    // Print the result sequentially
    for (i = 1; i <= MAX_SIEVE; i++)
    {
        int blockNr = i / 64;
        int bitpos = i - (blockNr * 64);
        int isPrimary = buffer[blockNr] & (1 >> bitpos) != 0;
        if (isPrimary)
        {
            printf("%d\n", i);
        }
    }
    return (0);
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid());
        first_call = false;
    }
    usleep (random () % t);
}

