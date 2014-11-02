
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
Thread distributer motor D3 = PD3
*/

unsigned long threadLostTime = 3000;

/*system speed*/
int speedState = 0;
int prevSpeedState = 1;
/*each motor own speed coefficent*/
float motorSpeedCoefs[5] = {1, 1, 1, 1, 1};

/*tension sensors functionality*/
boolean tensionProblem[5] = {0,0,0,0,0};
boolean tensionProblemLast[5] = {0,0,0,0,0};
unsigned long tensionDebounceStart[5] = {0,0,0,0,0};

/*helpers*/
byte portMask = 0x01;
int i, j, k=0;
boolean btSpeedUpState;
boolean btSpeedDownState;
boolean btStartStopState;
boolean btSpeedUpLastState;
boolean btSpeedDownLastState;
boolean btStartStopLastState;

/*input buttons as debounced library objecs*/
DebouncedButton btSpeedUp = DebouncedButton(8,50);
DebouncedButton btSpeedDown = DebouncedButton(12,50);
DebouncedButton btStartStop = DebouncedButton(7,50);


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
  
  digitalWrite(3, 0);
  digitalWrite(5, 0);
  digitalWrite(6, 0);
  digitalWrite(9, 0);
  digitalWrite(10, 0);
  digitalWrite(11, 0);
  
  DPL("outputs low");
  
}

void loop() {
  
/*Input buttons functionality*/
  btStartStopState = btStartStop.dbRead();
  btSpeedUpState = btSpeedUp.dbRead();
  btSpeedDownState = btSpeedDown.dbRead();
  
  if(!btStartStopState && btStartStopLastState) {
    if (speedState>0) {
      prevSpeedState = speedState;
      speedState = 0;
    }
    else {
      speedState = prevSpeedState;
    }
  }
  else if(!btSpeedDownState && btSpeedDownLastState) {
    if (speedState > 1) {
      speedState--;
      prevSpeedState = speedState;
    }
    else {
      prevSpeedState--;
    }
  }
  else if(speedState != 0 && !btSpeedUpState && btSpeedUpLastState) {//speed up button does not work while motors are stopped
    if (speedState < 10) {
      speedState++;
      prevSpeedState = speedState;
    }
  }
  if (prevSpeedState == 0) prevSpeedState = 1;
  
  btStartStopLastState = btStartStopState;
  btSpeedUpLastState = btSpeedUpState;
  btSpeedDownLastState = btSpeedDownState;

  
/*Tension sensors functionality*/
  portMask = 0x01;
  for (i=0; i<5; i++) {
    tensionProblemLast[i] = tensionProblem[i];
    
    if ((PINC & portMask) == 0) {
      tensionProblem[i] = 1;
      motorSpeedCoefs[i] = 1.25;
      if (tensionProblem[i] && !tensionProblemLast[i]) {
        tensionDebounceStart[i] = millis();
      }
    }
    else {
      tensionProblem[i] = 0;
      motorSpeedCoefs[i] = 1;
    }
    
    if (tensionProblem[i] == 1) {
      if (millis() - tensionDebounceStart[i] > threadLostTime) {
        prevSpeedState = speedState;
        speedState = 0;
        for (j=0; j<5; j++) {
          tensionProblem[j] = 0;
        }
        DPL("tension problem lasted long");
      }
    }
    
    portMask = portMask << 1;
  }
  
  
/*Write pwm according to each motors own speed coefficent*/
  analogWrite(11, 20*speedState*motorSpeedCoefs[0]);
  analogWrite(10, 20*speedState*motorSpeedCoefs[1]);
  analogWrite(9, 20*speedState*motorSpeedCoefs[2]);
  analogWrite(6, 20*speedState*motorSpeedCoefs[3]);
  analogWrite(5, 20*speedState*motorSpeedCoefs[4]);
  
  /*turn thread distributor on or off also*/
  analogWrite(3, speedState*20);
  
  if (k=10) {
    DP("System speed: ");
    DP(speedState);
    DP("  analogWrite values to motors: ");
    DP(int(20*speedState*motorSpeedCoefs[0]));
    DP("  ");
    DP(int(20*speedState*motorSpeedCoefs[1]));
    DP("  ");
    DP(int(20*speedState*motorSpeedCoefs[2]));
    DP("  ");
    DP(int(20*speedState*motorSpeedCoefs[3]));
    DP("  ");
    DPL(int(20*speedState*motorSpeedCoefs[4]));
    k=0;
  }
  k++;
  
  
  
  
  
  
}
