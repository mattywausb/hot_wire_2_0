/*

*/

#include "pitches.h"

// general game state
#define GST_BOOT 0
#define GST_TEST 1
#define GST_RUNNING 10
#define GST_WIN_INIT 20
#define GST_WIN_LOOP 21
#define GST_LOSS_INIT 30
#define GST_LOSS_LOOP 31

byte gameState = GST_BOOT;

#define GSL_COLLECT 1
#define GSL_BURN 2

byte gameSelect= GSL_COLLECT; 





/* current game setting */
unsigned long foul_duration=1000;
unsigned long foul_cooldown=1000;
int const foul_limit=8;

/* current game state */
#define FOUL_INITIATION 1
#define FOUL_COOLDOWN 2
#define ALL_GOOD 0
byte foulState=ALL_GOOD;
int foulCounter=0;
unsinged long foul_event_border=0;






// general technical constants
const long debounce_tolerance = 500; // milliseconds
int  portState=LOW;
int  analogHigh=false;
void setup() {

#define ANALOG_HIGH_THRESHOLD 512

}

void loop() {

  /* ---- Check all the input elements ---- */

 // Check areaA
  if(millis()>areaA_debounceComplete) {
        analogHigh=analogRead(areaA_pinA);
        if( analogHigh && !areaA_hasContact ) {
            areaA_hasContact=true;
            areaA_debounceComplete=millis()+debounce_tolerance;
        };
        if( !analogHigh && areaA_hasContact ) {
            areaA_hasContact=false;
            areaA_debounceComplete=millis()+debounce_tolerance;
    }
  }


  // Check wire
  if(millis()>wire_debounceComplete) {
        analogHigh=analogRead(wire_pinA)>ANALOG_HIGH_THRESHOLD;
        if( analogHigh && !wire_hasContact ) {
            wire_hasContact=true;
            wire_debounceComplete=millis()+debounce_tolerance;
        };
        if( !analogHigh && wire_hasContact ) {
            wire_hasContact=false;
            wire_debounceComplete=millis()+debounce_tolerance;
    }
  }

  // Check startButton 
  portState=digitalRead(bnStart_pinD);
  if( portState==HIGH && !bnStart_alreadyPressed )  { // We have a press event
        bnStart_alreadyPressed=true;
        bnStart_debounceComplete=millis()+debounce_tolerance;
        bnStart_hasPressEvent=true;
   } else { // LOW or already pressed
    bnStart_hasPressEvent=false;
   if(millis()>bnStart_debounceComplete && portState==LOW) {  // still LOW after debounce time
      bnStart_alreadyPressed=false;
    }
  }
  /* ------- Handling and Game Logic --------- */
  if(bnStart_hasPressEvent) {processGameSelection();};

  
  switch (gameState) {
    case GST_BOOT: initializeTest();
                   gameState=GST_TEST;
                   break;

    case GST_TEST: /* there are no states in Test mode to manage */ break;
    case GST_RUNNING: processGameStep(); break;
    case GST_LOSS: /* there are no states in loss mode to manage */ break;
    case GST_WIN: /* there are no states in win mode to manage */ break;
  }
 
  /* ------- Feedback -----------------*/

switch (gameState) {
    case GST_BOOT:
    case GST_TEST: if(micros()>output_nextSceneUpdateTime) {
                      output_scence_test_ledBar();
                      output_scence_test_sound();
                      output_nextSceneUpdateTime=micros()+output_sceneUpdateInterval;
                    }
                   rumble(wire_has_contact);
                   ledring_set(wire_has_contact);
                   break;
    case GST_RUNNING:
                   if(micros()>output_nextSequenceUpdateTime) {
                      output_scence_game_ledBar();
                      output_nextSequenceUpdateTime=micros()+output_sequenceUpdateInterval;
                    }
                   rumble_set(wire_has_contact);
                   ledring_set(wire_has_contact);
                   break;
    case GST_WIN_INIT:
                   output_sequence_win();
                   break;
    case GST_WIN_LOOP:
                   if(micros()>output_nextAnimationTime) { output_scene_win();};
    case GST_LOSS_INIT:
                   output_sequence_loss(); 
    case GST_LOSS_LOOP:
                   if(micros()>output_nextAnimationTime) output_scene_loss();
    
 }; // switch
                   
    
} //----------- End of  loop -------------------------

 /* System test functions */
 void initializeTest() {
   ledBar_animation_Test_start();
   sound_animationTest_start();
 }

  /* Game Logic Functions */
 void processGameSelection() {
    
     /* Read game select and set level parameters */
    byte selectReadout=0;
    if(digitalRead(SELECT_1_PIND)==HIGH) {bitSet(selectReadout,0)};
    if(digitalRead(SELECT_2_PIND)==HIGH) {bitSet(selectReadout,2)};
    if(digitalRead(SELECT_4_PIND)==HIGH) {bitSet(selectReadout,3)};

    if(selectReadout==0) {  initializeTest();}
    else { 
      gameState=GST_RUNNING;
      /* toDoset all parameters and initialize animations accordingly */
      foul_valid_border=0;
      foul_cooldown_border=0;
      foulState=ALL_GOOD;
      foulCounter=0;
      output_sequence_game_start();
    }
    
    return;
};

void processGameStep() {

  switch (foulState) {
    ALL_GOOD:   if(wire_hasContact) {
                  foulState=FOUL_INITIATION;
                  foul_event_border=micros()+foul_duration;
                  } else {
                  if(gameSelect ==GSL_BURN  && foulCounter>0 && micros()>foul_event_border) {
                      foulCounter-=1;
                      foul_event_border=micros()+foul_cooldown;
                    }
                  }
                  break;
    FOUL_INITIATION: 
                if(!wire_hasContact) {
                  foulState=ALL_GOOD;
                  foul_event_border=micros()+foul_cooldown;
                } ELSE {
                  if(micros()>foul_event_border) { // Count Foul and switch to cooldown
                    foulCounter+=1;
                    foulState=FOUL_COOLDOWN;
                    foul_event_border=micros()+foul_cooldown;
                  }
                }
    FOUL_COOLDOWN:
                if(micros()>foul_event_border) { // Wait for cooldown until initiating next foul
                    foulState=ALL_GOOD;
                 }
  }

  if(foulCounter>foul_limit) { gameState=GST_LOSS_INIT; } 
}


/* ------------------- output for the test state ------------------- */





/* Feedback  in the Running state */
void output_sequence_game_start() {
      rumble_set(false);
      ledring_set(false);
}
    

void output_scene_game_ledBar();


