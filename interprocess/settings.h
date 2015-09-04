#ifndef _SETTINGS_H_
#define _SETTINGS_H_

// remove the comments for the output you like: either graphical (X11) output
// or storage in a BMP file (or both)
#define WITH_X11
//#define WITH_BMP

// settings for interprocess communications
// (note: be sure that /proc/sys/fs/mqueue/msg_max >= MQ_MAX_MESSAGES)
#define NROF_WORKERS    64
#define MQ_MAX_MESSAGES 64

// settings for the fractal computations
#define INFINITY        10.0
#define MAX_ITER        512

// settings for graphics
#define X_PIXEL         880
#define Y_PIXEL         660
#define X_LOWERLEFT     -2.0
#define Y_LOWERLEFT     -1.0
#define STEP            0.003
//#define X_LOWERLEFT     -0.65
//#define Y_LOWERLEFT     -0.5
//#define STEP            0.0001

// lower left pixel (0,0) has coordinate
//                  (X_LOWERLEFT, Y_LOWERLEFT)
// upperright pixel (X_PIXEL-1,Y_PIXEL-1) has coordinate
//                  (X_LOWERLEFT+((X_PIXEL-1)*STEP),Y_LOWERLEFT+((Y_PIXEL-1)*STEP))

#endif

