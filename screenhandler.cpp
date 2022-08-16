//Make this readable and futureproofing
#define CurrPin A0
#define Cell1Pin A1
#define Cell2Pin A2
#define Cell3Pin A3

//Resistor values
#define Cell1R1 5100
#define Cell1R2 1500

#define Cell2R1 3600
#define Cell2R2 2000

#define Cell3R1 3000
#define Cell3R2 7500

//Buzzer pin
#define buzzPin 6

//Needed to store the boot logo
#include <avr/pgmspace.h>
//Screen
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

//"Multithreading"
#include "TeensyThreads.h"

//Heaader files
#include "screenhandler.h"
#include "logoHandler.h"

//Timer interrupts
#include <Metro.h>

//Select screen type. The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

//Instantiate an interrupt instance set to 5s
Metro notiMetro = Metro(5000);

//If the buzzer can beep. Only here so it won't spam
bool buzz_rdy = true;
//If a notification is avaliable
bool noti_avaliable;
//If battery is connected
bool BattConn;

void screenInit()
{
  Serial.begin(9600);
  
  u8g2.begin();           // Initialise screen
  u8g2.clearBuffer();
  
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(0);
  u8g2.drawXBM(20,10,56,56,bootlogo); // bootlogo[] stored in "logoHandler.h"
  u8g2.sendBuffer();
  delay(2000);
   
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.setFontMode(0);    // enable transparent mode, because speed
  
}

//Rebuild the main screen
void buildScreen()
{
  //Create positioning struct for easy positioning
  struct posBatVol {
    int x;
    int y;
  };
  posBatVol positVol[2];

  //Adjust text origin points here
  /*
                                    -------------
                                    |   BattX:  |
      adjustable coordinate here -> *           |
                                    |   XX.XX V |
                                    -------------
  */
  //Voltage
  positVol[0].x = 0;
  positVol[0].y = 15;

  positVol[1].x = 42;
  positVol[1].y = 15;

  positVol[2].x = 88;
  positVol[2].y = 15;
  
  //Current
  int CurrX = 10;
  int CurrY = 48;

  //Power
  int WattX = 78;
  int WattY = 48;

  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

  //Update warning status
  checkForWarnings();

  //Notify user if avaliable
  if (noti_avaliable)
  {
    //Place lines for separation
    drawLines();  
    //Start the buzzer thread
    threads.addThread(buzzer);
  }
  else
  {
    //Allow buzzer to run
    buzz_rdy = true;
    //Place lines for separation
    drawLines();
  }

  //If the timer elapsed, clear notifications
  if (notiMetro.check() == 1) {
    noti_avaliable = false;
  }

  //draw The voltages on the screen
  drawVolt("Cell1:", analogRead(Cell1Pin), Cell1R1, Cell1R2, positVol[0].x, positVol[0].y, 1);

  drawVolt("Cell2:", analogRead(Cell2Pin), Cell2R1, Cell2R2, positVol[1].x, positVol[1].y, 2);

  drawVolt("Cell3:", analogRead(Cell3Pin), Cell3R1, Cell3R2, positVol[2].x, positVol[2].y, 3);

  //Calculate current and power draw
  float current = getCurr(analogRead(CurrPin));
  float watt = getCurr(analogRead(CurrPin)) * getVolt(analogRead(Cell1Pin), Cell1R1, Cell1R2, 1);

  //Text buffer
  char str[8];
  char str2[8];
  
  //Convert to const char* and show on screen
  u8g2.drawStr(CurrX, CurrY, "Current:");
  dtostrf(current, 3, 2, str);
  u8g2.drawStr(CurrX + 5, CurrY + 10, str);
  u8g2.drawStr(CurrX + 30, CurrY + 10, "A");

  //Same here
  u8g2.drawStr(WattX, WattY, "Power:");
  dtostrf(watt, 3, 2, str2);
  u8g2.drawStr(WattX, WattY + 10, str2);
  u8g2.drawStr(WattX + 30, WattY + 10, "W");

  //When the buffer is ready, send to the screen and free up memory
  u8g2.sendBuffer();
}
/*
void buildVoltageScreen()
{
  if (BattConn)
  {
    //Create positioning struct for easy positioning
    struct posBatVol {
      int x;
      int y;
    };
    posBatVol positVol[2];
  
    //Adjust text origin points here
    //
    //                                  -----------------
    //                                  | BattX: XX.XXV |
    //   adjustable coordinate here ->  *----------------
    
    //Voltage
    positVol[0].x = 10;
    positVol[0].y = 20;
  
    positVol[1].x = 10;
    positVol[1].y = 40;
  
    positVol[2].x = 10;
    positVol[2].y = 60;
  
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_10x20_tf); //bigger font cus less info showed
  
    //Update warning status
    checkForWarnings();
  
    //Notify user if avaliable
    if (noti_avaliable)
    {
      //Start the buzzer thread
      threads.addThread(buzzer);
    }
    else
    {
      //Allow buzzer to run
      buzz_rdy = true;
    }
  
    //If the timer elapsed, clear notifications
    if (notiMetro.check() == 1) {
      noti_avaliable = false;
    }
  
    //draw The voltages on the screen
    drawVoltBig("Cell1:", analogRead(Cell1Pin), Cell1R1, Cell1R2, positVol[0].x, positVol[0].y, 1);
  
    drawVoltBig("Cell2:", analogRead(Cell2Pin), Cell2R1, Cell2R2, positVol[1].x, positVol[1].y, 2);
  
    drawVoltBig("Cell3:", analogRead(Cell3Pin), Cell3R1, Cell3R2, positVol[2].x, positVol[2].y, 3);
  
    //When the buffer is ready, send to the screen and free up memory
    u8g2.sendBuffer();
  }
  else
  {
    buildScreen();
  }
}
*/
void buildVoltageScreen()
{
  if (BattConn)
  {
    //Create positioning struct for easy positioning
    struct posBatVol {
      int x;
      int y;
    };
    posBatVol positVol[0];
  
    //Adjust text origin points here
    //
    //                                  -----------------
    //                                  | BattX: XX.XXV |
    //   adjustable coordinate here ->  *----------------
    
    //Voltage
    positVol[0].x = 0;
    positVol[0].y = 55;
  
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont( u8g2_font_fub35_tr ); //bigger font cus less info showed
  
    //Update warning status
    checkForWarnings();
  
    //Notify user if avaliable
    if (noti_avaliable)
    {
      //Start the buzzer thread
      threads.addThread(buzzer);
    }
    else
    {
      //Allow buzzer to run
      buzz_rdy = true;
    }
  
    //If the timer elapsed, clear notifications
    if (notiMetro.check() == 1) {
      noti_avaliable = false;
    }
  
    //draw The voltages on the screen
    drawVoltBig(analogRead(Cell1Pin), Cell1R1, Cell1R2, positVol[0].x, positVol[0].y, 0);
  
    //When the buffer is ready, send to the screen and free up memory
    u8g2.sendBuffer();
  }
  else
  {
    buildScreen();
  }
}



//Actually do the calculations and drawing of values
void drawVoltBig(float analogIn, float R1, float R2, int x, int y, int CellID)
{
  //Text buffer
  char str[8];
  //Calculations compensating for the voltage divider.
  float vin = getVolt(analogIn, R1, R2, CellID);

  //Draw relative to adjustable coordinates
  //If the voltage is below limit, show a warning
  if (vin < 1)
    {
      //Disconnected battery
      buildScreen();
    }
  else if (vin < 9.9)
    {
      u8g2.drawStr(x, y, "LOW!");
    }
  else
    {
      //If not, convert to const char* and show on screen
      dtostrf(vin, 3, 2, str); //Draw relative to coords
      u8g2.drawStr(x, y, str);
    }
}

//Actually do the calculations and drawing of values
void drawVolt(char* txt, float analogIn, float R1, float R2, int x, int y, int CellID)
{
  //Text buffer
  char str[8];
  //Calculations compensating for the voltage divider.
  float vin = getVolt(analogIn, R1, R2, CellID);
  
  //Draw relative to adjustable coordinates
  u8g2.drawStr(x, y, txt); //Draw on screen
  //If the voltage is below limit, show a warning
  if (vin < 1 && CellID == 1)
  {
    //Battery is NOT connected and don't draw the next screen (It's useless without this connected)
    BattConn = false;
    u8g2.drawStr(x, y + 10, "NC");
  }
  else if (vin < 1)
  {
    //Show this cell as NC
    u8g2.drawStr(x, y + 10, "NC");
  }
  else if (vin < 3.2)
  {
      //Battery is connected
      BattConn = true;
      u8g2.drawStr(x, y + 10, "LOW!");
  }
  else
  {
    //Battery is connected
    BattConn = true;  
    //If not, convert to const char* and show on screen
    dtostrf(vin, 3, 2, str); //Else, draw relative to coords
    u8g2.drawStr(x, y + 10, str);
    u8g2.drawStr(x + 28, y + 10, "V");
  }
}


//Calculate battery voltage and return as float
float getVolt(float analogIn, float R1, float R2, int CellID) {
  float val = analogIn; //read analog in
  float vout = (val * 3.3) / 1023.0; //Convert 10-bit to float voltage
  float vin = vout / (R2 / (R1 + R2)); //Ohms law
  
  
  
  //Calculate cell voltage by subtracting the other cell volatges. Recognise how much to subtract by identifying the current cell
  switch(CellID)
  {
    case 1:
      vin = vin - getVolt(analogRead(Cell2Pin), Cell2R1, Cell2R2, 2) - getVolt(analogRead(Cell3Pin), Cell3R1, Cell3R2, 3);
      break;
    case 2:
      vin = vin - getVolt(analogRead(Cell3Pin), Cell3R1, Cell3R2, 3);
      break;
    //This will cover cell ID 3 as well as 0 when you don't want any subtraction
    default:
      vin = vin;
      break;     
  }
  
  return vin;
}

//Calculate current based on shunt output
float getCurr(float analogIn) {
  float vout = (analogIn * 3.3) / 1023.0; //Convert 10-bit to float voltage
  float current = mapfloat(vout, 0.0, 2.3, 0.0, 2.4); //Calculate battery currentU

  return current;
}

//Update the warning buffer
void checkForWarnings()
{
  //Check for voltage warnings
  if (getVolt(analogRead(Cell1Pin), Cell1R1, Cell1R2, 1) <= 9.6)
  {
    noti_avaliable = true;
  }
  if (getVolt(analogRead(Cell2Pin), Cell2R1, Cell2R2, 2) <= 6.4)
  {
    noti_avaliable = true;
  }
  if (getVolt(analogRead(Cell3Pin), Cell3R1, Cell3R2, 3) <= 3.2)
  {
    noti_avaliable = true;
  }
}

//Buzzer thread, literally beep it 2 times and flag as unavaliable
void buzzer()
{
  //If buzzer is avaliable
  if (buzz_rdy)
  {
    pinMode(buzzPin, OUTPUT);
    digitalWrite(buzzPin, HIGH);
    delay(50);
    digitalWrite(buzzPin, LOW);
    delay(100);
    digitalWrite(buzzPin, HIGH);
    delay(50);
    digitalWrite(buzzPin, LOW);
    buzz_rdy = false;
  }
}

//It's map() but with floats
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

//Draw the lines seen on the main screen
void drawLines()
{
  u8g2.drawLine(38, 0, 38, 32);
  u8g2.drawLine(82, 0, 82, 32);
  u8g2.drawLine(0, 32, 128, 32);
  u8g2.drawLine(64, 32, 64, 64);
}
