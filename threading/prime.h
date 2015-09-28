/* 
 * Operating Systems  (2INC0)  Practical Assignment
 * Threaded Application
 *
 * Joris Geurts
 * j.geurts@fontys.nl
 *
 */

/**
 * NROF_SIEVE: size of the sieve
 * (value must be between 1 and 15485864 (such that primes.txt can be used for checking))
 */
#define NROF_SIEVE          4000

/**
 * NROF_THREADS: number of threads that will run in parallel (only for assignment "Threaded Application")
 */
#define NROF_THREADS        10

/**
 * buffer[]: datastructure of the sieve; each number is represented by one bit
 */
static unsigned long long   buffer [(NROF_SIEVE/64) + 1];
