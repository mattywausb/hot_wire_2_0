//#define TRACE_ON 

/* Functions to handle all input elements */

#include "mainSettings.h"

// Activate general trace output

#ifdef TRACE_ON
#define TRACE_INPUT 
//#define TRACE_INPUT_HIGH
//#define TRACE_INPUT_TIMING 
#endif

const unsigned long contact_debounce_tolerance = 500; // milliseconds


/* Button constants */ 
const byte digi_pin_list[] = {2,    // Mode Select 
                               };
#define INPUT_PORT_COUNT sizeof(digi_pin_list)
#define INPUT_CHANGE_COOLDOWN 10   // Milliseconds until we accept the next state change of an input

                               };

/* Button variables */

unsigned long buttons_last_read_time=0;  // Millis at last buttoncheck
unsigned int button_raw_state=0;         // state of buttons in last button check
unsigned int button_tick_state = 0;      // current and historized state in the actual tick

/* Generic button bit pattern (Button = 0) */

#define INPUT_0_BITS 0x0003
#define INPUT_0_IS_ON_PATTERN 0x0003
#define INPUT_0_SWITCHED_ON_PATTERN 0x0001
#define INPUT_0_SWITCHED_OFF_PATTERN 0x0002


/* Element specific  button bit pattern */
/*                                         76543210 */

#define INPUT_BUTTON_A_BITS                 0x0003
#define INPUT_BUTTON_A_IS_PRESSED_PATTERN   0x0003
#define INPUT_BUTTON_A_GOT_PRESSED_PATTERN   0x0001
#define INPUT_BUTTON_A_GOT_RELEASED_PATTERN  0x0002

#define INPUT_ALL_BUTTON_STATE_MASK 0x0003


/* history handling */

#define INPUT_CURRENT_BITS 0x0005
#define INPUT_PREVIOUS_BITS 0x000a


unsigned long last_press_start_time=0;
unsigned long last_press_end_time=0;
bool input_enabled=true;

/* General state variables */
unsigned long input_last_change_time = 0;


/* Analog Input Handling */
#define VOLTAGE_PIN 12
const byte zone_pin_list[] = {0,1,2 }    // hot wire, Landing 1, landing 2
#define ZONE_COUNT sizeof(ana_pin_list)
#define ANALOG_HIGH_THRESHOLD 512
#define ANALOG_COOLDOWN_INTERVAL 70 
unsigned long analog_prev_measure_time = 0;
unsigned long analog_contact_start_time = 0;
unsigned long analog_contact_end_time = 0;
int analog_current_zone=-1;



/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

/* ------------- Button events --------------- */

bool input_selectGotPressed()
{

  return input_enabled && ((button_tick_state & INPUT_BUTTON_A_BITS) == INPUT_BUTTON_A_GOT_PRESSED_PATTERN);
}

bool input_selectIsPressed()
{
  return input_enabled && ((button_tick_state & INPUT_BUTTON_A_BITS) == INPUT_BUTTON_A_IS_PRESSED_PATTERN); 
}

byte input_selectGotReleased()
{
  return input_enabled && ((button_tick_state & INPUT_BUTTON_A_BITS) == INPUT_BUTTON_A_GOT_RELEASED_PATTERN); 
}

long input_getCurrentPressDuration()
{
  #ifdef TRACE_INPUT_TIMING
    Serial.print(F("TRACE_INPUT_TIMING:input CurrentPressDuration:"));
    Serial.println(millis()-last_press_start_time);
  #endif
    
  return millis()-last_press_start_time;
}

long input_getLastPressDuration()
{
  return  last_press_end_time-last_press_start_time;
}


/* ------------- Operations ----------------- */



/* Disable input until all buttons have been released */

void input_IgnoreUntilRelease()
{
  input_enabled=false;
}


/* ************************************* TICK ***********************************************
   translate the state of buttons into the ticks of the master loop
   Must be called by the master loop for every cycle to provide valid event states of
   all input devices.
   Also transfers state changes, tracked with the timer interrupt into a tick state
*/

void input_switches_scan_tick()
{
  bool change_happened=false;
  
  /* put regular button scan into buttow_raw_state*/
  if (millis() - buttons_last_read_time > INPUT_BUTTON_COOLDOWN)
  {
    byte isPressed=0;
    int analog_value=0;
    buttons_last_read_time = millis();
    
    for (int i = 0; i <INPUT_PORT_COUNT; i++) { // for all input ports configured
      isPressed=!digitalRead(switch_pin_list[i]);
      bitWrite(button_raw_state,i*2,isPressed);  
    }
  }

  /* copy previous tick  state to history bits  and take raw pattern as new value */
  button_tick_state = (button_tick_state & INPUT_CURRENT_BITS) << 1
               | (button_raw_state & INPUT_CURRENT_BITS);

  
  /* Track pressing time */
  for (int i =0;i<INPUT_PORT_COUNT;i++)
  {
    if((button_tick_state & (INPUT_0_BITS<<(i*2))) == INPUT_0_SWITCHED_ON_PATTERN<<(i*2)) 
    {
      change_happened=true;
      last_press_end_time =last_press_start_time=millis();
      #ifdef TRACE_INPUT_HIGH
        Serial.print(("TRACE_INPUT_HIGH:press of "));Serial.println(i);
      #endif     
    }
    if((button_tick_state & (INPUT_0_BITS<<(i*2))) == INPUT_0_SWITCHED_OFF_PATTERN<<(i*2)) 
    {
      last_press_end_time=millis();
      change_happened=true;
      #ifdef TRACE_INPUT_HIGH
        Serial.print(("TRACE_INPUT_HIGH:release of "));Serial.println(i);
      #endif     
    }
  }

  if((button_tick_state & INPUT_ALL_BUTTON_STATE_MASK) ==0x00)  input_enabled=true; // enable input when all is released and settled

  if(change_happened)input_last_change_time = millis(); // Reset the globel age of interaction
} // void input_switches_tick()


/* essential wire test, returns number of the wire, when there is contact or -1 if there is no contact -2 if there are ambiguos readings */

int get_contact_zone()
{ 
   if(millis()-analog_prev_measure_time< ANALOG_COOLDOWN_INTERVAL) return analog_current_zone;  // Provide previous result until cooldown is over
   analog_prev_measure_time=millis();
   
   int zone;
   byte analogHigh;
   /* check for false contacts */
   digitalWrite(VOLTAGE_PIN,LOW);  
   for(c=0;c<ZONE_COUNT;c++) {      
      analogHigh=analogRead(c)>ANALOG_HIGH_THRESHOLD;
      if(analogHigh) return -2;   // since we dont provide voltage yet, there should be no reading
   }

   /* Now check for real contact */
   digitalWrite(VOLTAGE_PIN,HIGH);  
   for(zone=0;zone<ZONE_COUNT;c++) {      
      analogHigh=analogRead(zone)>ANALOG_HIGH_THRESHOLD;
      if(analogHigh) break;  
   }
   if(zone>=ZONE_COUNT) zone=-1;  // Set to -1 if no contact is detected;

   /* Manage the time tracking */
   if(zone!=analog_current_zone) {
      analog_contact_start_time=analog_contact_end_time=analog_prev_measure_time; 
      analog_current_zone=zone;
   } else {
     analog_contact_end_time=analog_prev_measure_time;
   }
   return analog_current_zone;
}

unsigned long get_contact_state_duration() 
{
  return analog_contact_end_time-analog_contact_start_time;
}

/* ***************************       S E T U P           ******************************
*/

void input_setup() {

  /* Initialize switch pins and raw_state_register array */
  for (byte switchIndex = 0; switchIndex < INPUT_PORT_COUNT ; switchIndex++) {
    pinMode(switch_pin_list[switchIndex], INPUT_PULLUP);
  }

  pinMode(VOLTAGE_PIN, OUTPUT); digitalWrite(VOLTAGE_PIN,LOW);

  analog_contact_start_time=analog_contact_end_time=0;
  
  setupComplete = true;
}

