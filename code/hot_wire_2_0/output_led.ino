#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "mainSettings.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT
//#define TRACE_HSV_HIGH
//#define TRACE_OUTPUT_HIGH
#endif

#define PIXEL_DATA_PIN 10
 // MOCKUP PIXEL STRIP definition
 

#define PIXEL_COUNT 8
#define PIXEL_BRIGHTNESS 100 // only 1/2 of amps necessary

unsigned long fade_start_time=0;
#define FADE_DURATION 1000
#define FADE_DURATION_F 2000.0

t_color_hsv hsv_yellow={60,1,1};
t_color_hsv hsv_green={120,1,1};
t_color_hsv hsv_red={0,1,1};
t_color_hsv hsv_border={0,1,1};


const int pixel_foul_amount=(FOUL_LIMIT/PIXEL_COUNT);

Adafruit_NeoPixel light_bar=Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

void output_led_setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    light_bar.begin(); 
    light_bar.setBrightness(PIXEL_BRIGHTNESS);         
    digitalWrite(LED_BUILTIN, false);                        
}

int output_get_pixel_foul_amount() {return pixel_foul_amount;}

/* ============= SCENES ============== */


/* ---- SETUP ---- */
void output_scene_setup()
{
  byte blink=(millis()/100)%2;
  for(int p=0;p<PIXEL_COUNT;p++) {
    switch(p){
    case 0:
        if(!blink) light_bar.setPixelColor(p, light_bar.Color(20,0,128));
        else light_bar.setPixelColor(p, light_bar.Color(0,0,0));
        break;
    case 7:
        if(blink) light_bar.setPixelColor(p, light_bar.Color(20,0,128));
        else light_bar.setPixelColor(p, light_bar.Color(0,0,0));
        break;
    case 3:
    case 4:
        if(input_get_contact_zone()==ZONE_HOT) light_bar.setPixelColor(p, light_bar.Color(255,255,0));
        else light_bar.setPixelColor(p, light_bar.Color(0,0,0));
        break;
    default:
        light_bar.setPixelColor(p, light_bar.Color(0,0,0));
    } // switch
  } // for
  output_led_show();
  digitalWrite(LED_BUILTIN,(millis()/100)%2);
}


/* ---- IDLE ---- */
void output_scene_idle()            
{
  byte blink= 255-((millis()/10)%255);
  for(int p=0;p<PIXEL_COUNT;p++) {
    if(p%3<2 && p/3<=g_current_difficulty) light_bar.setPixelColor(p, light_bar.Color(0,blink>>1,blink));
    else light_bar.setPixelColor(p, light_bar.Color(0,0,0));
  }
  output_led_show();
  digitalWrite(LED_BUILTIN,(millis()/2000)%2);
}



/* ---- GAME ---- */
void output_scene_game()             
{
  // Calculate border pixel
  int full_foul_counter=foul_counter+foul_current_cumulation;
  byte border_pixel=(full_foul_counter)/pixel_foul_amount;
  hsv_border.h=120.0-((120.0*(full_foul_counter%pixel_foul_amount))/pixel_foul_amount); //120 = green, 0 = red

  if(input_get_contact_zone()==ZONE_HOT) {  // contact feedback
    digitalWrite(LED_BUILTIN,HIGH);
    if(foul_running) {
        fade_start_time=millis();
        for(int p=0;p<PIXEL_COUNT;p++)  light_bar.setPixelColor(p, light_bar.Color(255,255,255));
        hsv_border.s=1;
        output_led_set_pixel_hsv(border_pixel,hsv_border);
    } else {
        for(int p=0;p<PIXEL_COUNT;p++)  light_bar.setPixelColor(p, light_bar.Color(255,255,0));    
    }
  } else { // not in contact = fade to normal display
    digitalWrite(LED_BUILTIN,LOW);
    unsigned long fade_time=millis()-fade_start_time;
  
  
    // Calculate saturation fading
    float saturation=1;
    if(fade_time<FADE_DURATION) saturation=(float)fade_time/FADE_DURATION;
    hsv_green.s=saturation;
    hsv_red.s=saturation;
    hsv_border.s=saturation;

    #ifdef TRACE_HSV_HIGH
        Serial.print(F(">TRACE_HSV_HIGH: border at "));
        Serial.print(border_pixel);
        Serial.print(F("-"));
        print_hsv_to_serial(hsv_border);
        Serial.println();
    #endif
  
    // Put colors to strip
    for(int p=0;p<PIXEL_COUNT;p++) {
      if(p<border_pixel) output_led_set_pixel_hsv(p,hsv_red);
        else if (p==border_pixel) output_led_set_pixel_hsv(p,hsv_border);
          else output_led_set_pixel_hsv(p,hsv_green);
    }
  }  // end if contact 
  output_led_show(); 
}


/* ---- WIN ---- */
void output_scene_win(unsigned long millis_since_start)              /* SCENE WIN */
{
  hsv_green.s=1;
  hsv_border.s=1;
  int border_pixel=foul_counter/pixel_foul_amount;
  hsv_border.h=120.0-((120.0*(foul_counter%pixel_foul_amount))/pixel_foul_amount); //120 = green, 0 = red

  byte blink=(millis()/150)%4;  // 4 state time value
  if(millis_since_start<5000) {    // green animation for 5 sec
    for(int p=4;p<PIXEL_COUNT;p++) {
       byte value=((4-p+blink)%4)*60;  // transform to value from 10-250  
       light_bar.setPixelColor(p, light_bar.Color(0,value,0));              
       light_bar.setPixelColor(PIXEL_COUNT-p-1, light_bar.Color(0,value,0));              
    }
  } else {                        // show final result
    for(int p=0;p<PIXEL_COUNT;p++) {
        if(p<border_pixel) light_bar.setPixelColor(p, light_bar.Color(0,0,0));
         else if (p==border_pixel) output_led_set_pixel_hsv(p,hsv_border);
            else output_led_set_pixel_hsv(p,hsv_green);
      }
  }
  output_led_show(); 
  digitalWrite(LED_BUILTIN,LOW);
}


/* ---- LOSS ---- */
void output_scene_loss(unsigned long millis_since_start)              /* SCENE LOSS */
{
  hsv_red.s=1;
  byte blink=(millis_since_start/150)%2;
  for(int p=0;p<PIXEL_COUNT;p++) {
    if(p%2==blink) output_led_set_pixel_hsv(p,hsv_red);
    else light_bar.setPixelColor(p, light_bar.Color(0,0,0));
  }
  output_led_show(); 
  digitalWrite(LED_BUILTIN,LOW);
}


/* ======== Untilities =========== */

/* set one pixel on the led  manually */
void output_led_set_pixel_color(byte pixel_index,int r, int g, int b)
{
  light_bar.setPixelColor(pixel_index, light_bar.Color(r,g,b));
}

/* set one pixel on the led  manually */
void output_led_set_pixel_hsv(byte pixel_index,t_color_hsv color_hsv)
{
  t_color_rgb rgb = get_color_rgb(color_hsv);
  light_bar.setPixelColor(pixel_index, light_bar.Color(rgb.r*255,rgb.g*255,rgb.b*255));
}

/* set all pixel on the led  manually */
void output_led_set_bar_hsv(t_color_hsv color_hsv)
{
  t_color_rgb rgb = get_color_rgb(color_hsv);
  output_led_set_bar_rgb(rgb);
}

/* set all pixel on the led   */
void output_led_set_bar_rgb(t_color_rgb rgb)
{
  for(int p=0;p<PIXEL_COUNT;p++) {
    light_bar.setPixelColor(p, light_bar.Color(rgb.r*255,rgb.g*255,rgb.b*255));
  }
}

void output_led_show()
{
  #ifdef TRACE_OUTPUT_HIGH
      Serial.println(F(">output_show"));
  #endif
    light_bar.show();                                  
}


t_color_rgb get_color_rgb(t_color_hsv color_hsv)
{
    float      hue_segment, p, q, t, hue_segment_remainder;
    long        hue_segment_int;
    t_color_rgb         out;

    if(color_hsv.s <= 0.0) {       // zero or negative saturation will result in pure value 
        out.r = color_hsv.v;
        out.g = color_hsv.v;
        out.b = color_hsv.v;
        return out;
    }
    hue_segment = color_hsv.h;
    while(hue_segment >= 360.0) hue_segment -=360.0;
    while(hue_segment <0) hue_segment +=360.0;
    hue_segment /= 60.0;
    hue_segment_int = (long)hue_segment;
    hue_segment_remainder = hue_segment - hue_segment_int;
    p = color_hsv.v * (1.0 - color_hsv.s); // whitening color element
    #define hsv_get_q   color_hsv.v * (1.0 - (color_hsv.s * hue_segment_remainder))
    #define hsv_get_t   color_hsv.v * (1.0 - (color_hsv.s * (1.0 - hue_segment_remainder)))

    switch(hue_segment_int) {
    case 0:   // pure red to pure yellow 0-60
        out.r = color_hsv.v;
        out.g = hsv_get_t;
        out.b = p;
        break;
    case 1:   // yellow to pure green   60-120
        out.r = hsv_get_q;
        out.g = color_hsv.v;
        out.b = p;
        break;
    case 2:   // green to cyan   120-180
        out.r = p;
        out.g = color_hsv.v;
        out.b = hsv_get_t;
        break;

    case 3:  // cyan to blue  180-240
        out.r = p;
        out.g = hsv_get_q;
        out.b = color_hsv.v;
        break;
    case 4: // blue to magenta  240-300
        out.r = hsv_get_t;
        out.g = p;
        out.b = color_hsv.v;
        break;
    case 5: // magenta to red  300-360
    default:
        out.r = color_hsv.v;
        out.g = p;
        out.b = hsv_get_q;
        break;
    }
    return out;     
}

void print_hsv_to_serial(t_color_hsv color)
{
  Serial.print(color.h);Serial.print(F("/"));
  Serial.print(color.s);Serial.print(F("/"));
  Serial.print(color.v);Serial.print(F("(hsv)"));
}

