/***************************************************************************
glcdbp.h

Header file for glcdbp.c and many, many other things around the project.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#ifndef __glcdbp_h
#define __glcdbp_h

#define BUF_DEPTH 256 // Ring buffer size. Originally set to 416.

// These typedefs will be used throughout the project to track the type of
//  display we're using as well as whether we want the pixel(s) at the heart
//  of a command to be turned on or off.
typedef enum DISPLAY_TYPE {SMALL, LARGE} DISPLAY_TYPE;
typedef enum PIX_VAL {ON, OFF} PIX_VAL;

void timerInit(void);

#endif
