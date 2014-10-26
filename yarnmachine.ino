#include <DebouncedButtons.h>
/*debug*/
#define DEBUG /*comment this line out in production*/
#include "debug.h"

/*LAYOUT
Tension sensors frol left to right:
A0,A1,A2,A3,A4 = port C 0-4
Start/stop button D7 = PD7
Speed up button D8 = PB0
Speed down button D12 = PB4
Motors from left to right
D5, D6 = PD5, PD6
D9, D10, D11 = PB1, PB2, PB3
*/

/*system speed*/
int speedState = 0;
int prevSpeedState = 0;
/*each motor own speed coefficent*/
int motorSpeedCoefs = [20, 20, 20, 20, 20];

/*tension sensors functionality*/
int tensionProblem = [0,0,0,0,0];
unsigned long tensionDebounceStart = [0,0,0,0,0];

/*helpers*/
byte portMask = 0x01;
int i;

/*input buttons as debounced library objecs*/
DebouncedButton btSpeedUp = DebouncedButton(3,50);
DebouncedButton btSpeedDown = DebouncedButton(3,50);
DebouncedButton btStartStop = DebouncedButton(7,50);


void setup() {
  
  /*debug*/#ifdef DEBUG
    /*debug*/Serial.begin(9600);
  /*debug*/#endif
  //DPL("setup");
  
  /*Pin modes*/
  DDRC = DDRC & ~0x1F;//tension sensors inputs
  DDRD = DDRD | 0x60;//motor outputs
  DDRB = DDRB | 0xE;//motor outputs
  //button inputs are handeled in library objects
  
  
}

void loop() {
  
/*Input buttons functionality*/

  if(btStartStop.dbRead()) {
    if (speedState>0) {
      prevSpeedState ) speedState;
      speedState = 0;
    }
    else {
      speedState = prevSpeedState;
    }
  }
  else if(btSpeedDown.dbRead()) {
    if (speedState > 0) {
      speedState--;
      prevSpeedState = speedState;
    }
  }
  else if(btSpeedUp.dbRead()) {
    if (speedState < 10) {
      speedState++;
      prevSpeedState = speedState;
    }
  }
  
/*Tension sensors functionality*/
  
  motorSpeedCoefs = [20, 20, 20, 20, 20];
  for (portMask = 0x01; portMask <= 0x1F; portMask << 1) {
    if (tensionProblem[i] == 1) {
      if (millis() - tensionDebounceStart[i] > threadLostTime) {
        speedState = 0;
        prevSpeedState = 0;
      }
    }
    if ((PINC & portMask) == 0) {
      tensionProblem[i] = 1;
      tensionDebounceStart[i] = millis();
      motorSpeedCoefs[i] = 22;
    }
    else {
      tensionProblem[i] = 0;
    }
    i++;
  }
  
  
/*Write pwm according to each motors own speed coefficent*/
  analogWrite(5, speedState*motorSpeedCoefs[0]);
  analogWrite(6, speedState*motorSpeedCoefs[1]);
  analogWrite(9, speedState*motorSpeedCoefs[2]);
  analogWrite(10, speedState*motorSpeedCoefs[3]);
  analogWrite(11, speedState*motorSpeedCoefs[4]);
  
  
}
