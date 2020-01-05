
#include "mainSettings.h"

#ifdef TRACE_ON
#define TRACE_MODES
//#define TRACE_SCORE
#endif

enum PROCESS_MODES {
  IDLE_MODE, 
  GAME_MODE,
  WIN_MODE,
  LOSS_MODE,
  SETUP_MODE
};


PROCESS_MODES g_process_mode = SETUP_MODE; 

/* current game setting */

#define FOUL_LIMIT 32000
#define RESTART_THRESHOLD 10000

int g_current_difficulty=1;

/* current game state */
unsigned long foul_detect_interval=0;
int foul_speed=0;
int foul_fallback_rate=0;
int foul_counter=0;
int foul_autoincrement=0;
unsigned long game_frame_start_time=0;
unsigned long foul_current_cumulation=0;
unsigned long mode_time_sync_register=0;
boolean foul_running=false;
int start_zone=-4;
int finish_zone=-4;

#define GAME_FRAME_DURATION 100

void setup() 
{

  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif

  
  output_led_setup();
  sound_setup();
  input_setup();

  enter_SETUP_MODE();
}

void loop() 
{
   input_switches_scan_tick();
   input_zone_scan_tick();
   switch(g_process_mode) {
    case SETUP_MODE: process_SETUP_MODE();break;
    case IDLE_MODE: process_IDLE_MODE();break;
    case GAME_MODE:process_GAME_MODE();break;
    case WIN_MODE:process_WIN_MODE();break;
    case LOSS_MODE:process_LOSS_MODE();break;
   } // switch
   sound_tick();
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
    foul_counter=0;
   
}

void process_SETUP_MODE()
{
  int zone=input_get_contact_zone();
  if(zone==ZONE_LANDING_1 || zone==ZONE_LANDING_2) {
    if (input_get_contact_state_duration()>100) {
      start_zone=zone;
      finish_zone= start_zone==ZONE_LANDING_1 ? ZONE_LANDING_2 : ZONE_LANDING_1;
      enter_IDLE_MODE();
      return;
    }
  }
  if(zone==ZONE_HOT) {
    sound_start_beep();
  } else {
   sound_stop();
  }
  output_scene_setup();
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
     sound_start_GameStartMelody();
}

void process_IDLE_MODE()
{
   if(input_selectGotPressed()) 
   {
      if(++g_current_difficulty>=3)g_current_difficulty=0;
      return;
   }

   if(input_get_contact_zone()==ZONE_NONE && input_get_contact_state_duration()>100) {
    enter_GAME_MODE();
    return;
   }
   output_scene_idle();
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
    unsigned long foul_detect_interval=0;
    foul_detect_interval=1;
    foul_speed=1000;
    foul_fallback_rate=0;
    foul_counter=0;
    foul_running=false;
    foul_autoincrement=0;
    foul_current_cumulation=0;
    game_frame_start_time=mode_time_sync_register=millis();
    switch (g_current_difficulty) {
      case 0:
        foul_detect_interval=500;
        foul_autoincrement=0;
        foul_speed=4;
        foul_fallback_rate=50;
        break;
      case 1:
        foul_detect_interval=300;
        foul_autoincrement=0;
        foul_speed=7;
        foul_fallback_rate=0;
        break;
      case 2:
        foul_detect_interval=150;
        foul_autoincrement=60;
        foul_speed=7;
        foul_fallback_rate=0;
        break;
    }
}

void process_GAME_MODE()
{
  if(input_get_contact_zone()==start_zone&& (millis()-mode_time_sync_register<RESTART_THRESHOLD || input_get_contact_state_duration()>RESTART_THRESHOLD)) {
    enter_IDLE_MODE();
    return;
  }
  
  if(input_get_contact_zone()==ZONE_HOT && input_get_contact_state_duration()>foul_detect_interval) {
    foul_running=true;
    foul_current_cumulation=(input_get_contact_state_duration()-foul_detect_interval)*foul_speed;
    
    #ifdef TRACE_SCORE
        Serial.print(F(">TRACE_SCORE"));
        Serial.println(foul_current_cumulation+foul_counter);
    #endif
  } else foul_running=false;

  if(input_get_contact_zone()==ZONE_HOT) {
    sound_start_beep();
  } else {
    sound_stop();
  }

  if(millis()-game_frame_start_time>GAME_FRAME_DURATION) {
    game_frame_start_time=millis();
    foul_counter+=foul_autoincrement;  // Manage autoincrement
    if(foul_fallback_rate>0) {          // Manage fallback
      int pixel_remainder=foul_counter%output_get_pixel_foul_amount();
      if( pixel_remainder>0) 
          if(foul_fallback_rate<pixel_remainder) foul_counter-=foul_fallback_rate;
          else foul_counter-=pixel_remainder;
    }
  }

  if(foul_current_cumulation+foul_counter>FOUL_LIMIT) {
      enter_LOSS_MODE();
      return;
  }
    
  if(input_get_contact_zone()!=ZONE_HOT && foul_current_cumulation>0) {
    foul_counter+=foul_current_cumulation;
    foul_current_cumulation=0;
    #ifdef TRACE_SCORE
        Serial.println(F(">TRACE_SCORE - contact ended"));
    #endif
  }

  if(input_get_contact_zone()==finish_zone) {
    enter_WIN_MODE();
    return;
  }

  output_scene_game();
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
    mode_time_sync_register=millis();
    sound_start_GameOverMelody();
}


void process_WIN_MODE()
{
  if(millis()-mode_time_sync_register > 15000 || input_selectGotPressed()) 
  {
    enter_SETUP_MODE();
  }

  output_scene_win(millis()-mode_time_sync_register);
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
    mode_time_sync_register=millis();
    sound_start_Crash();
}

void process_LOSS_MODE()
{
  if(millis()-mode_time_sync_register > 10000 || input_selectGotPressed()) 
  {
    enter_SETUP_MODE();
  }
  output_scene_loss(millis()-mode_time_sync_register);
}

/* ******************** Memory Helper *************** */
 
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
