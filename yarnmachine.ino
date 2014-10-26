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


DebouncedButton btSpeedUp = DebouncedButton(3,50);
DebouncedButton btSpeedDown = DebouncedButton(3,50);
DebouncedButton btStartStop = DebouncedButton(3,50);


void setup() {
  
  /*debug*/#ifdef DEBUG
    /*debug*/Serial.begin(9600);
  /*debug*/#endif
  DPL("setup");
  
  /*Pin modes*/
  DDRC = DDRC & ~0x1F;//tension sensors inputs
  DDRB = DDRB & ~0x11;//button inputs (speed up, down)
  DDRD = DDRD & ~0x80;//button input (startstop)
  DDRD = DDRD | 0x60;//motor outputs
  DDRB = DDRB | 0xE;//motor outputs
  
  
}

void loop() {
  
  boolean button1debounced = btSpeedUp.dbRead();
  boolean button1debounced = btSpeedDown.dbRead();
  boolean button1debounced = btStartStop.dbRead();
  
}
