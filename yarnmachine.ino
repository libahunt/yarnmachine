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

unsigned long threadLostTime = 5000;

/*system speed*/
int speedState = 0;
int prevSpeedState = 1;
/*each motor own speed coefficent*/
int motorSpeedCoefs[5] = {20, 20, 20, 20, 20};

/*tension sensors functionality*/
int tensionProblem[5] = {0,0,0,0,0};
unsigned long tensionDebounceStart[5] = {0,0,0,0,0};

/*helpers*/
byte portMask = 0x01;
int i, j;

/*input buttons as debounced library objecs*/
DebouncedButton btSpeedUp = DebouncedButton(8,300);
DebouncedButton btSpeedDown = DebouncedButton(12,300);
DebouncedButton btStartStop = DebouncedButton(7,300);


void setup() {
  
  /*debug*/#ifdef DEBUG
    /*debug*/Serial.begin(9600);
  /*debug*/#endif
  DPL("setup");
  
  /*Pin modes*/
  DDRC = DDRC & ~0x1F;//tension sensors inputs
  DDRD = DDRD | 0x60;//motor outputs
  DDRB = DDRB | 0xE;//motor outputs
  //button inputs are handeled in library objects
  
  digitalWrite(5, 0);
  digitalWrite(6, 0);
  digitalWrite(9, 0);
  digitalWrite(10, 0);
  digitalWrite(11, 0);
  
  DPL("ports low");
  
}

void loop() {
  
/*Input buttons functionality*/

  if(!btStartStop.dbRead()) {
    if (speedState>0) {
      prevSpeedState = speedState;
      speedState = 0;
    }
    else {
      speedState = prevSpeedState;
    }
  }
  else if(!btSpeedDown.dbRead()) {
    if (speedState > 0) {
      speedState--;
      prevSpeedState = speedState;
    }
  }
  else if(!btSpeedUp.dbRead()) {
    if (speedState < 10) {
      speedState++;
      prevSpeedState = speedState;
    }
  }
  if (prevSpeedState == 0) prevSpeedState = 1;
  DPL(speedState);
  
/*Tension sensors functionality*/
  for (i=0; i<5; i++) {
    motorSpeedCoefs[i] = 20;
  }
  portMask = 0x01;
  for (i=0; i<5; i++) {
    if (tensionProblem[i] == 1) {
      if (millis() - tensionDebounceStart[i] > threadLostTime) {
        speedState = 0;
        prevSpeedState = 1;
        for (j=0; j<5; j++) {
          tensionProblem[j] = 0;
        }
        return;
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
    portMask << 1;
  }
  
  
/*Write pwm according to each motors own speed coefficent*/
  analogWrite(11, speedState*motorSpeedCoefs[0]);
  DPL(speedState*motorSpeedCoefs[0]);
  analogWrite(10, speedState*motorSpeedCoefs[1]);
  analogWrite(9, speedState*motorSpeedCoefs[2]);
  analogWrite(6, speedState*motorSpeedCoefs[3]);
  analogWrite(5, speedState*motorSpeedCoefs[4]);
  
  
}
