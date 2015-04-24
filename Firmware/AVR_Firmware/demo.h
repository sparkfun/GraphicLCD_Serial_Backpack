/***************************************************************************
demo.h

Header file for demo mode. Includes function prototypes and some flash
 stored constant strings.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#ifndef __demo_h
#define __demo_h

#include <avr/pgmspace.h> 

static char string_1[] PROGMEM = "Well here we are again.\0";
static char string_2[] PROGMEM = "It's always such a pleasure.\0";
static char string_3[] PROGMEM = "Remember how you tried to kill me twice?\0";
static char string_4[] PROGMEM = "Oh how we laughed and laughed!\0";
static char string_5[] PROGMEM = "Except I wasn't laughing.\0";
static char string_6[] PROGMEM = "Under the circumstances, I've been SHOCKINGLY nice.\0";

static PGM_P wantYouGone[] PROGMEM =
{
  string_1,
  string_2,
  string_3,
  string_4,
  string_5,
  string_6  
};

void    demo(void);

#endif