//This will handle the screen drawing and calculations
#include "screenhandler.h"

//Timer interrupts
#include <Metro.h>

//Instantiate an interrupt set to 5s
Metro serialMetro = Metro(5000);

//Which screen should be showed (Main/voltage only)
int screen = 0;

void setup() {
  //Initialise screen
  screenInit();

  //MCU status LED
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  //buzzer
  pinMode(6, OUTPUT);
}

void loop() {
  //If the timer elapsed, show the next screen and loop
  if (serialMetro.check() == 1) {
  screen++;
    if (screen == 2)
    {
      screen = 0;
    }
  }

  //Rebuild screen to the selected one
  //All the calculations will be handled on the spot while building
  switch (screen)
  {
    case 0:
    //Full details
      buildScreen();
      break;
    case 1:
    //Only the sum of voltages in a big font
      buildVoltageScreen();
      break;
  }
}
