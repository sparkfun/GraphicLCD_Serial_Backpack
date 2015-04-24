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
	void setBacklight(byte duty);
	void setBaud(byte baud);
	void restoreDefaultBaud();
  	void setX(byte posX);
	void setY(byte posY);
	void setHome();
	void demo();
	void setPixel(byte x, byte y, byte set);
	void drawLine(byte x1, byte y1, byte x2, byte y2, byte set);
	void drawBox(byte x1, byte y1, byte x2, byte y2, byte set);
	void drawCircle(byte x, byte y, byte rad, byte set);
	void eraseBlock(byte x1, byte y1, byte x2, byte y2);
	
	
	private:
    
    
    

};

#endif


