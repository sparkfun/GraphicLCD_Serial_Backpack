/***************************************************************************
ui.h

Header file for the user interface. Describes the various commands, what
 they do, and how to access them. Establishes some nice, readable constants
 for the various commands.

02 May 2013 - Mike Hord, SparkFun Electronics

This code is released under the Creative Commons Attribution Share-Alike 3.0
 license. You are free to reuse, remix, or redistribute it as you see fit,
 so long as you provide attribution to SparkFun Electronics.

***************************************************************************/

#ifndef __ui_h
#define __ui_h

/*
  This header file specifies the user interface functionality of the device.
  The UI is handled via serial communications, with a specific sequence of
  characters as defined below:
  '|'            (0x7c) - Command sequence start. Follow with one of the
                            additional characters below to execute a command.
  'CTRL-SHIFT-2' (0x00) - Clear screen.
  'CTRL-d'       (0x04) - Execute demo code.
  'CTRL-r'       (0x12) - Toggle light-on-dark/dark-on-light mode. Nonvolatile.
                            All draw commands operate relative to this setting,
                            drawing the opposite color against the background.
  'CTRL-s'       (0x13) - Toggle splash on startup. Nonvolatile.
  'CTRL-b'       (0x02) - Change backlight level. Next byte sent indicates the
                            desired backlight level as an unsigned binary value
                            from 0-100%.
  'CTRL-g'       (0x07) - Change interface baud rate. The next byte sent
                            indicates desired new baud rate:
                            '1' = 4800bps
                            '2' = 9600bps
                            '3' = 19200bps
                            '4' = 38400bps
                            '5' = 57600bps
                            '6' = 115200bps (default)
  'CTRL-x'       (0x18) - Change the text cursor x position
  'CTRL-y'       (0x19) - Change the text cursor y position
                            For both of these commands, the next byte reflects
                            the upper left corner of the text generation
                            cursor. The cursor will be incremented by one
                            6-pixel x motion after each character typed, and
                            if a character would extend partly past the edge of
                            the screen, the line will wrap. Backspace is also
                            implemented.
  'CTRL-p'       (0x10) - Set/clear a random pixel on the screen. The next byte
                            is an unsigned binary x value, followed by unsigned
                            binary y value, and 0x00 to clear or 0x01 to set
                            the pixel at that location.
  'CTRL-l'       (0x0c) - Draw (or erase) pixels in a line. Expects five bytes
                            after the command: two sets of x,y coordinates, and
                            a 0x00 or 0x01 for erase/draw.
  'CTRL-c'       (0x03) - Draw (or erase) pixels in a circle. Expects four
                            bytes: x,y of center, radius, and 0x00 or 0x01 for
                            erase or draw.
  'CTRL-o'       (0x0F) - Draw (or erase) the boundary lines of a box. Expects
                            five more bytes: two sets of x,y coordinates and
                            0x00 or 0x01 for erase/draw. The coordinates
                            represent the ends of a line drawn diagonally
                            across the box.
  'CTRL-e'       (0x05) - Erase a block. Expects four bytes: two sets of x,y
                            coordinates. These define the start and end
                            points of a diagonal line across the box; pixels
                            inside that box (including the border) will be
                            set to the background color.
  'CTRL-k'       (0x0b) - Draw a sprite. Expects five bytes- x and y of upper
                            left corner of the 8x8 sprite (unlike text, there
                            is no wrapping or edge detection- pixels off screen
                            won't render, but the sprite may be drawn partly
                            off screen), the index of the sprite, an angle,
                            which is defined by an ASCII character ('0' is 
                            un rotated, '3' is 90 deg clockwise, '6' is upside
                            down, and '9' is 90 deg anticlockwise), and last is
                            a zero or non-zero byte for erase or draw pixels.
*/

// These defines associate the above commands with cases in the switch
//  statement in the code in the c file.
#define  CLEAR_SCREEN   0x00
#define  RUN_DEMO       0x04
#define  TOGGLE_BGND    0x12
#define  TOGGLE_SPLASH  0x13
#define  ADJ_BL_LEVEL   0x02
#define  ADJ_BAUD_RATE  0x07
#define  ADJ_TEXT_X     0x18
#define  ADJ_TEXT_Y     0x19
#define  DRAW_PIXEL     0x10
#define  DRAW_LINE      0x0c
#define  DRAW_CIRCLE    0x03
#define  DRAW_BOX       0x0f
#define  ERASE_BLOCK    0x05
#define  DRAW_SPRITE    0x0b

#define  BL_LEVEL OCR1B // Just an alias, to make it more obvious what
                        //  we're doing when we write OCR1B is setting the
                        //  backlight level.

void      uiStateMachine(char command);

#endif
