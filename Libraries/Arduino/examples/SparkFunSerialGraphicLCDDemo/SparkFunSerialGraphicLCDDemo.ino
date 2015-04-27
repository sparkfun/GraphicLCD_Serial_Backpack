/*********************************************************
Serial Graphic LCD Library Demo
Joel Bartlett
SparkFun Electronics
October 14, 2013

License: 
This code is beerware: feel free to use it, with or without attribution, 
in your own projects. If you find it helpful, buy me a beer next time you 
see me at the local pub.

This is an example sketch to accompany the Serial Graphic LCD Library. 
This is intended for use with SparkFun's Serial Graphic LCDs: the 128x64 pixel,
the 160x128 pixel, and the Graphic LCD Serial Backpack. There is no guarantee 
that this code will work on any other LCDs or backpacks. 

This code was written with Arduino 1.0.5 (available at arduino.cc) It should 
work with any Arduino IDE version from 1.0 and on. If you are using 
Arduino 0023 or an older version, this will not compile.

Hardware:
You'll need one of the LCDs mentioned above as well as an Arduino compatible 
board running at 5V, and some jumper wires to connect the two. 

Connections are as follows:
__________________
Arduino | Backpack|
________|_________|
   5V   |  Vin    |
  GND   |  GND    |
   TX   |  RX     |
__________________

***********************************************************/

#include <SparkFunSerialGraphicLCD.h>//inculde the Serial Graphic LCD library
#include <SoftwareSerial.h>
//Despite this being inculdeing in the library file, 
//it needs to be added here as well to get the sketch to compile


//This demo code was created for both the 128x64 and the 160x128 pixel LCD. 
//Thus, these maximum limits are set to allow this code to work on both sizes of LCD.
//The default size is the 128x64, but you can delete those values and uncomment
//the other values if you have a 160x128 pixel LCD. If you are writing code for just 
//one size LCD, you can omit this information entirely and just use numerical values
//in place of these variable names. 

#define maxX 127//159 
#define maxY 63 //127
//Each maximum value is one less than the stated value to account for position 0,0
//Thus, position 127 is actually the 128th pixel. 

//Create an instance of the LCD class named LCD. We will use this instance to call all the 
//subsequent LCD functions
LCD LCD;

void setup()
{
//we're just going to run through a bunch of demos to show the functionality of the LCD.

delay(1200);///wait for the one second spalsh screen before anything is sent to the LCD.

LCD.setHome();//set the cursor back to 0,0.
LCD.clearScreen();//clear anything that may have been previously printed ot the screen.
delay(10);
  
LCD.printStr("Commence Arduino Demo Mode");
delay(1500);

//Each of these functions is explained in great detail below. 
helloWorld();

reverseModeDemo();

backlightDemo();

baudDemo();

counter();

tempAndHumidity();

XYDemo();

pixelDemo();

lineDemo();

boxDemo();

circleDemo();

eraseDemo();

demoDemo();

//This function should only be used if you accidentally changed the baud rate or if you forgot to what rate it was changed. 
//LCD.restoreDefaultBaud();
}
//-------------------------------------------------------------------------------------------
void loop()
{
//nothing in loop since we are just running through each demo once

}
//-------------------------------------------------------------------------------------------
void reverseModeDemo()
{
//Each LCD has a reverse mode built into it. The toggleReverseMode() function swaps the colors of the background and the pixels.
//For example, on the large display, the text is usually white on a blue background, but you could swap that to blue text on a white background. 
//This setting is stored and will remian even after power has been disconnected. 
  
LCD.setHome();
LCD.clearScreen();
LCD.printStr("Toggle reverse mode");
delay(2000);
  
LCD.clearScreen();
LCD.toggleReverseMode();
LCD.printStr("Reverse Mode On");
delay(1000);

LCD.clearScreen();
LCD.toggleReverseMode();
LCD.printStr("Reverse Mode Off");
delay(1000);
}
//-------------------------------------------------------------------------------------------
void backlightDemo()
{
//This function shows the different brightnesses to which the backlight can be set using the setBacklight() function.
//You can choose any number from the 0-100 range. If you are having trouble seeing text
//at different brightnesses, try adjusting the trimpot on the backpack. 
LCD.clearScreen();
LCD.printStr("Change the backlight brightness");
delay(2000);
LCD.clearScreen();

  for(int i = 0; i <= 100; i+=10)// 0-100 are the levels from off to full brightness
  {
      LCD.setBacklight(i);
      delay(100);
      LCD.printStr("Backlight = ");
      LCD.printNum(i);
      delay(500);
      LCD.clearScreen();
  }
}
//-------------------------------------------------------------------------------------------
void baudDemo()
{
//This function uses the setBaud() function to change the baud rate of the backpack.
//The deafult rate is 115200bps.
//If you loose track of what baud rate your LCD is set to, you can use the restoreDefaultBaud() 
//function to restore it back to 115200.
  
LCD.clearScreen();
LCD.printStr("This changes the Baud rate");
delay(2000);

LCD.clearScreen();
LCD.printStr("115200 is the Default rate");
delay(1500);
LCD.clearScreen();
LCD.printStr("Hello @ 115200");
delay(1000);
LCD.setBaud(53);//set to 57600

LCD.clearScreen();
LCD.printStr("Hello @ 57600");
delay(1000);
LCD.setBaud(52);//set to 38400

LCD.clearScreen();
LCD.printStr("Hello @ 38400");
delay(1000);
LCD.setBaud(51);//set to 19200

LCD.clearScreen();
LCD.printStr("Hello @ 19200");
delay(1000);
LCD.setBaud(50);//set to 9600 

LCD.clearScreen();
LCD.printStr("Hello @ 9600");
delay(1000);
LCD.setBaud(49);//set to 4800 

LCD.clearScreen();
LCD.printStr("Hello @ 4800");
delay(1000);
LCD.setBaud(54);//set back to 115200

LCD.clearScreen();
LCD.printStr("and back to 115200");
delay(1000);
}
//-------------------------------------------------------------------------------------------
void counter()
{
//Prints a simple counter to the LCD.
  LCD.clearScreen();
  LCD.printStr("Print variable info such as time");
  delay(2000);
  
  LCD.clearScreen();
  for(int i = 0; i <= 10; i++)
  {
    LCD.printStr("Counter = ");
    LCD.printNum(i);
    delay(500);
    LCD.clearScreen();
  }
}
//-------------------------------------------------------------------------------------------
void tempAndHumidity()
{
//This function shows how you could read the data from a temerature and humidity 
//sensor and then print that data to the Graphic LCD.
  
  LCD.clearScreen();
  LCD.printStr("Or, data from sensors");
  delay(2000);

  //these could be varaibles instead of static numbers 
  float tempF = 77.0; 
  float tempC = 25.0;
  float humidity = 67.0;
  
  LCD.clearScreen();
  
  LCD.printStr("Temp = ");
  LCD.printNum(int(tempF));
  LCD.printStr("F ");
  LCD.printNum(int(tempC));
  LCD.printStr("C");
  LCD.nextLine();
  LCD.printStr("Humidity = ");
  LCD.printNum(int(humidity)); 
  LCD.printStr("%");
  delay(2500);
}
//-------------------------------------------------------------------------------------------
void XYDemo()
{
//This function shows off the setX() and setY() functions. These two functions set the position of the cursor.
//That cursor sets where text will be printed.
//The parameters are simply the x and y value you want with each respective function. 

//Characters are 8 pixels tall x 6 pixels wide
//The top left corner of a char is where the x/y value will start its print
//For example, if you print a char at position 1,1, the bottom right of your char will be at position 7,9.
//Therefore, to print a character in the very bottom right corner, you would need to print at the coordinates 
//x = 154 , y = 120. You should never exceed these values. ***NOTE*** - these values are for the larger size LCD. 

//Here we have an example using an upper case 'B'. The star is where the character starts, given a set 
//of x,y coordinates. # represents the blocks that make up the character, and _ represnets the remaining 
//unused pixels in the character space. 
//    *###__
//    #   #_
//    #   #_
//    ####__
//    #   #_
//    #   #_
//    ####__
//    ______

  LCD.setHome();
  LCD.clearScreen();
  LCD.printStr("Change X and Y coordinates");
  delay(3000);
  
  LCD.clearScreen();
  LCD.setX((maxX/2)-9);//these are calculated by dividing maxX by 2, then subtracting (half of the number of chars in the word times 6).
  LCD.setY(0);
  LCD.printStr("Top");
  delay(1000);
  
  LCD.setX((maxX/2)-18);
  LCD.setY((maxY/2)-4);
  LCD.printStr("Middle");
  delay(1000);
  
  LCD.setX((maxX/2)-18);
  LCD.setY(maxY - 7);
  LCD.printStr("Bottom");
  delay(1000);
  
  LCD.setX(0);
  LCD.setY((maxY/2)-4);
  LCD.printStr("Left");
  delay(1000);
  
  LCD.setX(maxX - 30);
  LCD.setY((maxY/2)-4);
  LCD.printStr("Right");
  delay(1000);
  
  LCD.setX(maxX - 5);
  LCD.setY(maxY - 7);
  LCD.printStr("B");
  delay(1000);
  
  //Print # signs around the screen
  LCD.clearScreen();
  
  LCD.setY(0);
  for(int i=0;i<=(maxX - 5);i+=6)
  {
  LCD.setX(char(i));
  LCD.printStr("#");
  delay(10);
  }
  
  LCD.setY(maxY - 7);
  for(int i=0;i<=(maxX - 5);i+=6)
  {
  LCD.setX(char(i));
  LCD.printStr("#");
  delay(10);
  }
  
  LCD.setX(maxX - 5);
  for(int i=0;i<=(maxY - 7);i+=8)
  {
  LCD.setY(char(i));
  LCD.printStr("#");
  delay(10);
  }
  
  LCD.setX(0);
  for(int i=0;i<=(maxY - 7);i+=8)
  {
  LCD.setX(0);
  LCD.setY(char(i));
  LCD.printStr("#");
  delay(10);
  }
  delay(2000);
  
}
//-------------------------------------------------------------------------------------------
void pixelDemo()
{
//This function shows the setPixel() function. Any individual pixel on the LCD can be set or reset.
//Set means it's the color of the text depending on the reverse mode setting.
//Reset means it's the color of the background.
//The parameters for this function are (x, y, set) where x and y are the coordinates of the pixel you want to set or reset

  LCD.clearScreen();
  LCD.setHome();
  LCD.printStr("Control each pixel");
  delay(2000);

  LCD.clearScreen();

  //draw a box around the screen
  for(int i = 0; i <= maxX; i ++)
  {
    LCD.setPixel(i,0,1);
  }

  for(int i = 0; i <= maxY; i ++)
  {
    LCD.setPixel(maxX,i,1);
  }
  
    for(int i = maxX; i >= 0; i --)
  {
    LCD.setPixel(i,maxY,1);
  }
  
    for(int i = maxY; i >= 0; i --)
  {
    LCD.setPixel(0,i,1);
  }
  
  //draw vertical lines
  for(int i = 0, j = 0; i <= maxY; i++, j++)
  {
      LCD.setPixel(i,j,1);
  }
  
  for(int i = 0, j = maxY; i <= maxY; i++, j--)
  {
      LCD.setPixel(i,j,1);
  }
  
  for(int i = maxX, j = maxY; i >= 0; i--, j--)
  {
      LCD.setPixel(i,j,1);
  }
  
  for(int i = maxX, j = 0; i >= 0; i--, j++)
  {
      LCD.setPixel(i,j,1);
  }
  
  //draw a small box in the center
  LCD.setPixel((maxX/2),(maxY/2),1);
  LCD.setPixel(((maxX/2)+1),(maxY/2),1);
  LCD.setPixel((maxX/2),((maxY/2)+1),1);
  LCD.setPixel(((maxX/2)+1),((maxY/2)+1),1);
  delay(1000);
  

}
//-------------------------------------------------------------------------------------------
void lineDemo()
{
//This function demonstartes the drawLine() function. The parameters are (x1, y1, x2, y2, set).
//Lines can be drawn from any point to any other point within the boundaries of the LCD. 
  LCD.clearScreen();
  LCD.printStr("You can also draw lines");
  delay(1500);
  LCD.clearScreen();
  //draw x
  LCD.drawLine(0,0,maxX,maxY,1);//draw line from top left to bottom right
  delay(500);
  LCD.drawLine(0,maxY,maxX,0,1);//draw line from top left to bottom right
  delay(500);
  
  //draw box
  LCD.drawLine(0,0,maxX,0,1);//draw line from top left to top right
  delay(500);
  LCD.drawLine(0,maxY,maxX,maxY,1);//draw line from bottom left to bottom right
  delay(500);
  LCD.drawLine(0,0,0,maxY,1);//draw line from top left to bottom left
  delay(500);
  LCD.drawLine(maxX,0,maxX,maxY,1);//draw line from top right to bottom right
  delay(3000);

}
//-------------------------------------------------------------------------------------------
void boxDemo()
{
//This function demonstartes the drawBox() function. The parameters are (x1, y1, x2, y2, set).
  LCD.clearScreen();
  LCD.printStr("Or, just draw a box");
  delay(1500);
  LCD.clearScreen();

  LCD.drawBox(0,0,maxX,maxY,1);//draw box around entire display
  delay(1000);
  
  LCD.drawBox(10,10,(maxX - 10),(maxY - 10),1);//draw box around entire display
  delay(1000);

  LCD.drawBox(20,20,(maxX - 20),(maxY - 20),1);//draw box around entire display
  delay(1000);
  
  LCD.drawBox(30,30,(maxX - 30),(maxY - 30),1);//draw box around entire display
  delay(3000);
}
//-------------------------------------------------------------------------------------------
void circleDemo()
{
//This function demonstartes the drawCircle() function. The parameters are (x1, y1, radius, set).
  
  LCD.clearScreen();
  LCD.printStr("Circles are just as easy");
  delay(1500);
  LCD.clearScreen();
  
  LCD.drawCircle(5,(maxY/2),5,1);//draw box around entire display
  delay(500);
  LCD.drawCircle(15,(maxY/2),10,1);//draw box around entire display
  delay(500);
  LCD.drawCircle(30,(maxY/2),15,1);//draw box around entire display
  delay(500);
  LCD.drawCircle(50,(maxY/2),20,1);//draw box around entire display
  delay(500);
  LCD.drawCircle(75,(maxY/2),25,1);//draw box around entire display
  delay(500);
  LCD.drawCircle(105,(maxY/2),30,1);//draw box around entire display
  delay(500);
  LCD.drawCircle(140,(maxY/2),35,1);//draw box around entire display
  delay(3000);
}
//-------------------------------------------------------------------------------------------
void eraseDemo()
{
//This function demonstartes the eraseBlock() function. The parameters are (x1, y1, x2, y2).
  LCD.setHome();
  LCD.eraseBlock(0,0,maxX,16);//draw box around entire display
  delay(500);
  
  LCD.printStr("Last, earse part of the screen");
  delay(1000);
  
  
  LCD.eraseBlock(20,20,(maxX - 20),(maxY - 20));//draw box around entire display
  delay(500);
  
  LCD.drawBox(20,20,(maxX - 20),(maxY - 20),1);//draw box around entire display
  delay(1000);
  
  LCD.setX((maxX/2)-9);
  LCD.setY((maxY/2)-9);
  
  LCD.printStr("THE");
  
  LCD.setX((maxX/2)-9);
  LCD.setY((maxY/2)+2);
  
  LCD.printStr("END");
  delay(500);
  
  LCD.eraseBlock(21,21,(maxX - 21),(maxY - 21));//draw box around entire display
  delay(1000);
  
  LCD.setX((maxX/2)-6);
  LCD.setY((maxY/2)-9);
  
  LCD.printStr("OR,");
  
  LCD.setX((maxX/2)-18);
  LCD.setY((maxY/2)+2);
  
  LCD.printStr("IS IT?");
  delay(1000);
  
  
}
//-------------------------------------------------------------------------------------------
void helloWorld()
{
//Prints Hello World to the screen and draws a tiny world (circle)  
  
LCD.setHome();
LCD.clearScreen();
delay(10);

  LCD.setX((maxX/2)-15);
  LCD.setY(0);
  LCD.printStr("Hello");
  delay(250);
  
  
  LCD.setX((maxX/2)-15);
  LCD.setY(maxY - 7);
  LCD.printStr("World");
  delay(500);

  if(maxX == 159)
  {
   LCD.drawCircle((maxX/2),(maxY/2),40,1);
   delay(1000);
  }
  else
  {
   LCD.drawCircle((maxX/2),(maxY/2),20,1);
   delay(1000);
  }
}
//-------------------------------------------------------------------------------------------
void demoDemo()
{
//There is an internal demo feature built into the LCD firmware. Tjis function uses the demo() function to start that demo. 
//NOTE: Once the demo is initiated, there is no stopping it until it finishes or until power is disconnected from the LCD.
LCD.setHome();
LCD.clearScreen();
delay(10);

LCD.demo();
}
