/* 
 * Operating Systems  (2INC0)   Practical Assignment
 * Threaded Application
 *
 * Roel Hospel (0809845)
 * Mark Hendriks (0816059)
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

static const int BLOCK_COUNT = ((NROF_SIEVE / 64) + 1);

static const unsigned long long ONE = 1;

typedef struct {
    int base;
    sem_t* sem;
} Data;

typedef struct {
    pthread_t threads[NROF_THREADS];
    pthread_mutex_t* lock;
    int index;
} List;

static void rsleep (int t);

List* list;

static List* getList()
{
    static char init = 0;
    if (!init)
    {
        list = malloc(sizeof(List));
        list->lock = malloc(sizeof(pthread_mutex_t));
        list->index = 0;
        int errno;
        if ((errno = pthread_mutex_init(list->lock, NULL)) < 0)
        {
            printf("An error occurred whilst creating a mutex lock: %d\n", errno);
            exit(-1);
        }
        init = 1;
    }
    return list;
}

static void register_dead_thread(pthread_t value)
{
    List* list = getList();
    pthread_mutex_lock(list->lock);
    list->threads[list->index++] = value;
    pthread_mutex_unlock(list->lock);
}

static void flush_dead_threads()
{
    List* list = getList();
    pthread_mutex_lock(list->lock);
    int i;
    for (i = 0; i < list->index; i++)
    {
        pthread_join(list->threads[i], NULL);
    }
    list->index = 0;
    pthread_mutex_unlock(list->lock);
}

pthread_mutex_t* locks[(NROF_SIEVE / 64) + 1];

static pthread_mutex_t* getLock(int blockNr)
{
    static char initialized = 0;
    if (!initialized)
    {
        int  i;
        for (i = 0; i < BLOCK_COUNT; i++)
        {
            locks[i] = malloc(sizeof(pthread_mutex_t));
            pthread_mutex_init(locks[i], NULL);
        }
        initialized = 1;
    }
    // Return the address to lock for block blockNr
    return locks[blockNr];
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
static int scrap(int blockNr, unsigned long long val)
{
    lock(blockNr);
    unsigned long long old = buffer[blockNr];
    buffer[blockNr] = buffer[blockNr] & ~val;
    int changed = buffer[blockNr] != old;
    unlock(blockNr);
    return changed;
}

// Entry point for threads. Scraps all the numbers which are a multiple of the base.
static void* run_sieve(void* args)
{
    Data* data = args;
    unsigned long long val = 0;
    int base = data->base;
    int blockNr = base / 64;
    int n;
    for (n = base * 2; n <= NROF_SIEVE; n = n + base)
    {
        // n is twice the base, so it cannot be 0 (as base is guaranteed to be >= 1)
        if (n / 64 != blockNr)
        {
            scrap(blockNr, val);
            // Reset val
            val = 0;
            // Update blockNr
            blockNr = n / 64;
            rsleep(100);
        }
        // The value of 2 is at the 2nd bit
        // Which means we only need to bitshift 1 to the left
        // Hence the '- 1' at the end.
        unsigned long long bitpos = n - (blockNr * 64) - 1;
        val = val | (ONE << bitpos);
    }
    scrap(blockNr, val);
    register_dead_thread(pthread_self());
    sem_post(data->sem);
    free(data);
    return (NULL);
}

int main (void)
{
    // TODO: start threads generate all primes between 2 and NROF_SIEVE and output the results
    // (see thread_malloc_free_test() and thread_mutex_test() how to use threads and mutexes,
    //  see bit_test() how to manipulate bits in a large integer)
    // Initialize buffer
    int i;
    for (i = 0; i < BLOCK_COUNT; i++)
    {
        buffer[i] = -1;
    }

    // Initialize block locks and semaphore
    getLock(0);
    // Initialize List
    getList();
    sem_t sem;
    sem_init(&sem, 0, NROF_THREADS);

    // For each number within 1 and MAX_SIEVE
    for (i = 2; i <= NROF_SIEVE; i++)
    {
        // Wait for the semaphore to ensure we don't use more than NROF_THREAD
        sem_wait(&sem);
        flush_dead_threads();
        Data* data = malloc(sizeof(Data));
        data->base = i;
        data->sem = &sem;
        pthread_t thread;
        // Start the thread with the given data
        int errno;
        if ((errno = pthread_create(&thread, NULL, run_sieve, data)) < 0)
        {
            printf("ERROR: %d", errno);
            exit(1);
        }
        rsleep(100);
    }

    // Wait until all semaphore locks are for the main thread
    // As we don't want to store NROF_THREAD pthread_t structs,
    // I went with this (cheaper) version
    int n = NROF_THREADS;
    while (n--)
    {
        sem_wait(&sem);
    }
    sem_destroy(&sem);
    // Print the result sequentially
    for (i = 2; i <= NROF_SIEVE; i++)
    {
        int blockNr = i / 64;
        // The value of 2 is at the 2nd bit
        // Which means we only need to bitshift 1 to the left
        // Hence the '- 1' at the end.
        int bitpos = i - (blockNr * 64) - 1;
        // Check if the bit is set
        int isPrimary = (buffer[blockNr] & (ONE << bitpos)) != 0;
        if (isPrimary)
        {
            printf("%d", i);
            if (i < NROF_SIEVE) 
            {
                printf("\n");
            }
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

