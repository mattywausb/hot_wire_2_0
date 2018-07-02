
// Feedback channel states
#define LEDBAR_DATA_PIND 0
#define LEDBAR_CLOCK_PIND 1
#define LEDBAR_ENABLE_PIND 2

int ledBar_frameIndex=0;
unsigned long ledBar_nextAnimationTime=0;
const unsigned long ledBar_animationInterval=40; //25fps
const unsigned long ledBar_flashInterval=500; //0,5s
const unsigned long ledBar_chaserStepInterval=100; //0,1s

#define LEDRING_PIND 12
unsigned long ledRing_nextAnimationTime=0;

#define SOUND_PIND 11
int sound_frameIndex=0;
unsigned long sound_nextAnimationTime=0;

#define RUMBLE_PIND 13



/* --------------- General ouput processing functions --------------------*/

void ledBar_show(byte thePattern) {  /* Displays byte pattern on led bar */
   digitalWrite(LEDBAR_ENABLE_PIND,LOW);
   shiftOut(LEDBAR_DATA_PIND,LEDBAR_CLOCK_PIND,LSBFIRST,thePattern)
   digitalWrite(LEDBAR_ENABLE_PIND,HIGH);
};


/* -------- Output for the test mode -----------------*/
void output_sequence_test_start(){
  noTone(SOUND_PIND);  
  sound_frameIndex=0;
}


void output_scence_test_ledBar() {
    // LED ORDER: GGGGYYRR
    static const led_2_green = B11000000; 
    static const led_2_red=B00000011;
    static const led_2_yellow=B00011000;
  
    static byte led_pendulum=B00000111; // 00000111
    static bool pendulum_up=true;
    static unsigned long next_shift_time=0;

    // show the approprate effect
    if(areaA_hasContact) { ledBar_show(led_2_green) }
    else if(areaB_hasContact) { ledBar_show(led_2_red) }
    else if(wire_hasContact) { ledBar_show(led_2_yellow) }
    else ledBar_show(led_pendulum);

    // prepare next frame
    if(micros()>next_shift_time) {
      next_shift_time=micros()+ledBar_chaserStepInterval; 
      if(pendulum_up) {led_pendulum <<1; if(led_pendulum==B11100000) {pendulum_up=false;})
      else {led_pendulum >> 1; if(led_pendulum==B00000111) {pendulum_up=true;})
    }
    ledBar_nextAnimationTime=micros()+ledBar_animationInterval;  
}


void output_scence_test_sound() {  // play short beep when landing on area B
    
    if(areaB_hasContact) {
      if(sound_frameIndex==0) {
          tone(SOUND_PIND, NOTE_A3, 500);
          sound_frameIndex=1;
      }
    }else { sound_frameIndex=0;}
}
