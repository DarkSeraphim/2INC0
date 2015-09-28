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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq

#include "settings.h"
#include "output.h"
#include "common.h"

typedef struct {
    int id;
    pid_t pid;
    mqueue out;
    char* mqname;
} Worker;

typedef struct {
    mqueue in;
    char* mqname;
} Processor;

static void writeMessage(mqueue* dest, MessageS2C* out);

static void writeStatus(mqueue* dest, int status);

static int ppid;

static int getNextWorkerId()
{
    static int id = 0;
    return id++;
}

static int getNumberCount(int num)
{
    int i = 1;
    while (num /= 10)
    {
        i++;
    }
    return i;
}

static Processor* getReader() 
{
    Processor* processor = malloc(sizeof(Processor));
    struct mq_attr attr;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof (Message);
    int len = sizeof(char) * strlen(QUEUE_NAME) + (int)(getNumberCount(ppid) + 1);
    processor->mqname = malloc(len + 3);
    sprintf(processor->mqname, QUEUE_NAME, ppid, "in", 0);
    processor->in = mq_open(processor->mqname, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (processor->in < 0)
    {
        printf("[Farmer] Error when opening incoming queue: %s\n", strerror(errno));
        exit(1);
    }
    return processor;
}

static void initialize(Worker* worker) 
{
    int wid = worker->id = getNextWorkerId();
    int len = sizeof(char) * strlen(QUEUE_NAME) + (int)(getNumberCount(wid) + 1);
    char* destName = malloc(len + 4);
    sprintf(destName, QUEUE_NAME, ppid, "out", wid);
    char* srcName = malloc(len + 3);
    sprintf(srcName, QUEUE_NAME, ppid, "in", 0);

    struct mq_attr attr;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof (Message);
    worker->out = mq_open(destName, O_WRONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (worker->out < 0)
    {
        printf("[Farmer] Error when opening outgoing queue: %s\n", strerror(errno));
        exit(1);
    }
    worker->mqname = destName;
    int pid = worker->pid = fork();
    ASSERT(pid >= 0, "[Farmer] fork() failed");
    if (pid == 0) 
    {
        execlp("./worker", destName, srcName, NULL);
        perror("[Farmer] Failed to invoke execlp()");
    }
}

static void destroy(Worker* worker)
{
    mq_close(worker->out);
    mq_unlink(worker->mqname);
    free(worker->mqname);
}

static void dispatchTask(Worker* worker, int x, int y)
{
    MessageS2C message;
    message.x = x;
    message.y = y;
    writeMessage(&(worker->out), &message);
}

static void stopWorker(Worker* worker)
{
    writeStatus(&(worker->out), 0);
}

static void draw(int x, int y, int colour) 
{
    output_draw_pixel(x, y, colour);
}

static int readMessage(mqueue* src, Message* in) 
{
    int i = mq_receive(*src, (char*) in, sizeof(*in), NULL);
    if (i < 0)
    {
        printf("[Farmer] Error with receiving: %s\n", strerror(errno));
        exit(1);
    }
    return i;
}

static void writeMessage(mqueue* dest, MessageS2C* out)
{
    Message message;
    message.type = MESSAGE;
    message.data.serverMessage = *out;
    int i = mq_send(*dest, (char*) &message, sizeof(message), 0);
    if (i < 0)
    {
        printf("[Farmer] Error with sending message: %s\n", strerror(errno));
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
        printf("[Farmer] Error with sending status: %s\n", strerror(errno));
        exit(1);
    }
}

int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "[Farmer] %s: invalid arguments\n", argv[0]);
    }
    // TODO:
    //  * create the message queues & the children
    //  * do the farming (use output_draw_pixel() for the coloring)
    //  * wait until the children have been stopped
    //  * clean up the message queues
    ppid = getpid();
    Processor* processor = getReader();
    int pid = fork();
    if (pid == 0)
    {
        Worker workers[NROF_WORKERS];
        int i, j;
        for (i = 0; i < NROF_WORKERS; i++)
        {
            initialize(&workers[i]);
        }
        int workerIndex = 0;
        for (i = 0; i < X_PIXEL; i++) 
        {
            for (j = 0; j < Y_PIXEL; j++) 
            {
                // Optimize this. Let it record the amount of messages in queue
                workerIndex++;
                workerIndex %= NROF_WORKERS;
                dispatchTask(&workers[workerIndex], i, j);
            }
        }
        for (i = 0; i < NROF_WORKERS; i++)
        {
            stopWorker(&workers[i]);
            destroy(&workers[i]);
        }
        int status;
        int child;
        int n = NROF_WORKERS;
        while (n)
        {
            child = wait(&status);
            child++; // Suppress child unused warning for now
            n--;
        }
        free(processor->mqname);
        free(processor);
    } 
    else 
    {
        output_init ();
        int status;
        Message message;

        #ifdef DEBUG
        int count = 0;
        int max = X_PIXEL * Y_PIXEL;
        int per = max / 100;
        printf("[Farmer] Expecting %d pixels in debug batches of %d\n", max, per);
        #endif
        int n = NROF_WORKERS;
        while (readMessage(&processor->in, &message))
        {
            if (message.type == STATUS && !(--n))
            {
                break;
            }
            MessageC2S cMessage = message.data.clientMessage;
            draw(cMessage.x, cMessage.y, cMessage.result);
            #ifdef DEBUG
            count++;
            if (count % per == 0)
            {
                printf("[Farmer] Processed %2.2f of the pixels\n", ((float) count) / max);
            }
            #endif
        }
        #ifdef DEBUG
        printf("[Farmer] Processed a total of %d pixels\n", count);
        #endif
        mq_close(processor->in);
        mq_unlink(processor->mqname);
        free(processor->mqname);
        free(processor);
        output_end();
        int child = wait(&status);
        child++; // Suppress child unused warning for now
    }  
    return (0);
}

