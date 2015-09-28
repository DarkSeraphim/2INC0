/* 
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
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
#include <string.h>
#include <errno.h>          // for perror()
#include <unistd.h>         // for getpid()
#include <time.h>           // for time()
#include <complex.h>

#include "settings.h"
#include "common.h"

static int readMessage(mqueue* src, Message* in);
static void writeMessage(mqueue* dest, MessageC2S* out);
static void writeStatus(mqueue* dest, int status);
static void rsleep(int t);

static void pixel2coord (int x, int y, double* cx, double* cy)
{
    *cx = X_LOWERLEFT + STEP * ((double) x);
    *cy = Y_LOWERLEFT + STEP * ((double) y);
}

static double complex_dist (complex a)
{
    // distance of vector 'a'
    // (in fact the square of the distance is computed...)
    double re, im;
    
    re = __real__ a;
    im = __imag__ a;
    return ((re * re) + (im * im));
}

static int mandelbrot_point (double x, double y)
{
    int     k;
    complex z;
	complex c;
    
	z = x + y * I;     // create a complex number 'z' from 'x' and 'y'
	c = z;

	for (k = 0; (k < MAX_ITER) && (complex_dist (z) < INFINITY); k++)
	{
	    z = z * z + c;
    }
    
    //                                    2
    // k >= MAX_ITER or | z | >= INFINITY
    
    return (k);
}

int main (int argc, char * argv[])
{
    // TODO:
    //  * open the two message queues (whose names are provided in the arguments)
    //  * repeatingly:
    //      - read from a message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do that job (use mandelbrot_point() if you like)
    //      - write the results to a message queue
    //    until there are no more jobs to do
    //  * close the message queues
    mqueue src = mq_open(argv[0], O_RDONLY);
    if (src < 0)
    {
        printf("[Worker] Error with src mq_open: %s\n", strerror(errno));
        exit(1);
    }
    mqueue dest = mq_open(argv[1], O_WRONLY);
    if (src < 0)
    {
        printf("[Worker] Error with dest mq_open: %s\n", strerror(errno));
        exit(1);
    }

    Message in;
    MessageC2S out;
    while (readMessage(&src, &in)) 
    {
        if (in.type == STATUS)
        {
            writeStatus(&dest, 0);
            break;
        }
        rsleep(100);
        MessageS2C message = in.data.serverMessage;
        double x = 0, y = 0;
        pixel2coord(message.x, message.y, &x, &y);
        int result = mandelbrot_point(x, y);
        out.x = message.x;
        out.y = message.y;
        out.result = result;
        writeMessage(&dest, &out);
    }
    mq_close(src);
    mq_close(dest);
    return (0);
}

static int readMessage(mqueue* src, Message* in) 
{
    int i = mq_receive(*src, (char*) in, sizeof (*in), NULL);
    if (i < 0)
    {
        printf("[Worker] Error with receiving: %s\n", strerror(errno));
        exit(1);
    }
    return i;
}

static void writeMessage(mqueue* dest, MessageC2S* out)
{
    Message message;
    message.type = MESSAGE;
    message.data.clientMessage = *out;
    int i = mq_send(*dest, (char*) &message, sizeof(message), 0);
    if (i < 0)
    {
        printf("[Worker] Error with receiving: %s\n", strerror(errno));
        exit(1);
    }
}

static void writeStatus(mqueue* dest, int status)
{
    Message message;
    message.type = STATUS;
    message.data.status = status;
    int i = mq_send(*dest, (char*) &message, sizeof(message), 0);
    if (i < 0)
    {
        printf("[Worker] Error with receiving: %s\n", strerror(errno));
        exit(1);
    }
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time(NULL) % getpid());
        first_call = false;
    }
    usleep (random () % t);
}


