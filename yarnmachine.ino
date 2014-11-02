
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

/*If problem with thrread is detected then respective motor speeds up and after 
threadLostTime passes and the problem has not gone away then the machine will stop*/
unsigned long threadLostTime = 3000;//time in MILLISECONDS

/*System speed is set with SpeedUp and SpeedDown buttons
If machine is stopped (by buttonpress or thread breakage detection) then 
"start" buttonspress resumes the previous speed.
While machine is stopped, the SpeedDown button works, but SpeedUp does not.*/
int speedState = 0; //System speed has value from 0 to 10
int prevSpeedState = 1; //Stores previous speed for resuming

/*Button states needed for detecting beginning of a push*/
boolean btSpeedUpState;
boolean btSpeedDownState;
boolean btStartStopState;
boolean btSpeedUpLastState;
boolean btSpeedDownLastState;
boolean btStartStopLastState;

/*Each motor has it's own speed coefficent that is 1 usually but goes to 1.25
when thread problem is detected*/
float motorSpeedCoefs[5] = {1, 1, 1, 1, 1};

/*tension sensors functionality: states and problem start time*/
boolean tensionProblem[5] = {0,0,0,0,0};
boolean tensionProblemLast[5] = {0,0,0,0,0};
unsigned long tensionDebounceStart[5] = {0,0,0,0,0};

/*helpers*/
byte portMask = 0x01;
int i, j, k=0;


/*input buttons as debounced library objecs*/
DebouncedButton btSpeedUp = DebouncedButton(8,50);
DebouncedButton btSpeedDown = DebouncedButton(12,50);
DebouncedButton btStartStop = DebouncedButton(7,50);


void setup() {
  
  /*debug*/#ifdef DEBUG
    /*debug*/Serial.begin(9600);
  /*debug*/#endif
  
  /*Pin modes*/
  DDRC = DDRC & ~0x1F;//tension sensors inputs
  DDRD = DDRD | 0x60;//motor outputs
  DDRB = DDRB | 0xE;//motor outputs
  //button inputs are handeled in library objects
  

  
}

void loop() {
  
/*** Input buttons functionality ***/

  //read current button states
  btStartStopState = btStartStop.dbRead();
  btSpeedUpState = btSpeedUp.dbRead();
  btSpeedDownState = btSpeedDown.dbRead();
  
  //detect falling edges only
  if(!btStartStopState && btStartStopLastState) {
    if (speedState >0 ) {//machine is running - stop the machine
      prevSpeedState = speedState;
      speedState = 0;
    }
    else {
     speedState = prevSpeedState;//machine not running - turn on the machine, resume last speed 
    }
  }
  else if(!btSpeedDownState && btSpeedDownLastState) {
    if (speedState > 1) {//if machine is running reduce speed
      speedState--;
      prevSpeedState = speedState;
    }
    else {//if machine is stopped reduce the speed that will be resumed later
      prevSpeedState--;
    }
  }
  else if(speedState != 0 && !btSpeedUpState && btSpeedUpLastState) {//speed up button does not work while motors are stopped
    if (speedState < 10) {
      speedState++;
      prevSpeedState = speedState;
    }
  }
  if (prevSpeedState == 0) prevSpeedState = 1;//resumable speed has to be higher than 0
  
  //save button states for next loop
  btStartStopLastState = btStartStopState;
  btSpeedUpLastState = btSpeedUpState;
  btSpeedDownLastState = btSpeedDownState;

  
  
/*** Tension sensors functionality ***/

  portMask = 0x01;//for scanning over port C 0-4
  for (i=0; i<5; i++) {
    
    if ((PINC & portMask) == 0) {//has tension problem
    
      tensionProblem[i] = 1;
      motorSpeedCoefs[i] = 1.25;//increase speed coefficent to balance possible loose thread
      if (tensionProblem[i] && !tensionProblemLast[i]) {//if tension problem just started, save the time
        tensionDebounceStart[i] = millis();
      }
      
      if (millis() - tensionDebounceStart[i] > threadLostTime) {//if tension problem has been on for long
        prevSpeedState = speedState;//save speed state for resuming later
        speedState = 0;//stop the machine
        for (j=0; j<5; j++) {
          tensionProblem[j] = 0;//clear problem states
        }
      }
      
    }
    
    else {//all good
      tensionProblem[i] = 0;
      motorSpeedCoefs[i] = 1;//regular speed
    }
    
    tensionProblemLast[i] = tensionProblem[i];//save tension problem state
    portMask = portMask << 1;//move to next sensor
    
  }
  
  
/*** Write pwm according to each motors own speed coefficent ***/

  analogWrite(11, 20*speedState*motorSpeedCoefs[0]);
  analogWrite(10, 20*speedState*motorSpeedCoefs[1]);
  analogWrite(9, 20*speedState*motorSpeedCoefs[2]);
  analogWrite(6, 20*speedState*motorSpeedCoefs[3]);
  analogWrite(5, 20*speedState*motorSpeedCoefs[4]);
  
/** turn thread distributor on or off also ***/
  analogWrite(3, speedState*20);
  
  
/*** Print some debugging numbers to serial ***/ 

#ifdef DEBUG

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
  
#endif
  
  
  
}
