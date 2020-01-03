/*

*/

#include "pitches.h"

enum PROCESS_MODES {
  IDLE_MODE, 
  GAME_MODE,
  WIN_MODE,
  LOSS_MODE,
  SETUP_MODE
};


PROCESS_MODES g_process_mode = SETUP_MODE; 

/* current game setting */

unsigned long foul_duration=1000;
unsigned long foul_cooldown=1000;
int const foul_limit=8;

int g_current_difficulty=0;

/* current game state */
int foul_counter=0;
unsinged long foul_start_time=0;
int start_zone=-4;
int finish_zone=-4;


void setup() {

  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif
  output_led_setup();
  input_setup();

  enter_SETUP_MODE();
}

void loop() {

   input_switches_scan_tick();
   switch(g_process_mode) {
    case IDLE_MODE: process_IDLE_MODE();break;
    case GAME_MODE:process_GAME_MODE();break;
    case WIN_MODE:process_WIN_MODE();break;
    case LOSS_MODE:process_LOSS_MODE();break;
    case WAIT_MODE:process_WAIT_MODE();break;
   } // switch
   
} 

/* ======== SETUP_MODE ========*/

/* Wait until loop gets placed on one landing contact */

void enter_SETUP_MODE()
{
   #ifdef TRACE_MODES
      Serial.print(F("#SETUP_MODE: "));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif
    g_process_mode=SETUP_MODE;
}

void process_SETUP_MODE()
{
  int zone=get_contact_zone();
  if(zone==ZONE_LANDING_1 || zone==ZONE_LANDING_2) {
    if (get_contact_state_duration()>1000) {
      start_zone=zone;
      finish_zone= start_contact==ZONE_LANDING_1 ? ZONE_LANDING_2 : ZONE_LANDING_1;
      enter_IDLE_MODE();
      return;
    }
  }

  // Todo Play "Setup" Animation
}
/* ======== IDLE_MODE ========*/

void enter_IDLE_MODE()
{
   #ifdef TRACE_MODES
      Serial.print(F("#IDLE_MODE: "));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif
    g_process_mode=IDLE_MODE;
}

void process_IDLE_MODE()
{
   if(input_selectGotPressed()) 
   {
      if(++g_current_difficulty>=3)g_current_difficulty=0;
      return;
   }

   if(get_contact_zone()==ZONE_NONE && get_contact_state_duration()>1000) {
    enter_GAME_MODE();
    return;
   }

   // Todo play idle animation
}

/* ======== GAME_MODE ========*/

void enter_GAME_MODE()
{
   #ifdef TRACE_MODES
      Serial.print(F("#GAME_MODE: "));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif
    g_process_mode=GAME_MODE;
    /* define all paramters according to difficulty */
}

void process_GAME_MODE()
{
  /* start zone = back to idle or beep */
  /* Hot - count up and transotion to loss */
  /* finish zone = win */
  
}

/* ======== WIN_MODE ========*/

void enter_WIN_MODE()
{
   #ifdef TRACE_MODES
      Serial.print(F("#WIN_MODE: "));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif
    g_process_mode=WIN_MODE;
}


void process_WIN_MODE()
{
  /* Play animation for given time */
  /* switch to setup */
}

/* ======== LOSS_MODE ========*/

void enter_LOSS_MODE()
{
   #ifdef TRACE_MODES
      Serial.print(F("#LOSS_MODE: "));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif
    g_process_mode=LOSS_MODE;
}

void process_LOSS_MODE()
{
  /* Play animation for given time */
  /* switch to setup */
}


