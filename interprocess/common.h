/* 
 * Operating Systems  (2INC0)  Practical Assignment
 * Interprocess Communication
 *
 * Roel Hospel (0809845)
 * Mark Hendriks (0816059)
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <mqueue.h>         // for mq
#include "settings.h"

#define QUEUE_NAME "/mandelbrot-%d-0809845-and-0816059-%s-%d"

#define ASSERT(expr,msg) \
 do                  \
 {                   \
    if (!(expr))     \
    {                \
        perror(msg); \
        exit(-1);    \
    }                \
 } while (0);

typedef mqd_t mqueue;

typedef enum {
    STATUS, MESSAGE
} MessageType;

typedef struct {
    int x, y;
} MessageS2C;

typedef struct {
    int x, y, result;
} MessageC2S;

typedef struct {
    MessageType type;
    union {
        MessageC2S clientMessage;
        MessageS2C serverMessage;
        int status;
    } data;
} Message;

#endif

