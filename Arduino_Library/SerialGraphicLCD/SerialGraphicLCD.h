/**********************************************
Graphic Serial LCD Libary Header File
Joel Bartlett
SparkFun Electronics 
9-25-13

**********************************************/

#ifndef LCD_h
#define LCD_h

#include "Arduino.h"


class LCD
{
	public:
	LCD();
	void printStr(char Str[78]);
	void printNum(int num);
	void nextLine();
	void clearScreen();
    void toggleReverseMode();
	void toggleSplash();
	void setBacklight(int duty);
	void setBaud(char baud);
	void restoreDefaultBaud();
  	void setX(char posX);
	void setY(char posY);
	void setHome();
	void demo();
	void setPixel(int x, int y, int set);
	void drawLine(byte x1, byte y1, byte x2, byte y2, byte set);
	void drawBox(int x1, int y1, int x2, int y2, int set);
	void drawCircle(int x, int y, int rad, int set);
	void eraseBlock(int x1, int y1, int x2, int y2);
	
	
	private:
    
    
    

};

#endif


