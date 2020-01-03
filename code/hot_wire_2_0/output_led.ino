#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "mainSettings.h"

#ifdef TRACE_ON
//#define TRACE_OUTPUT
//#define TRACE_OUTPUT_HIGH
#endif

#define PIXEL_DATA_PIN 10
 // MOCKUP PIXEL STRIP definition
 

#define NUMPIXELS 8
#define PIXEL_BRIGHTNESS 210 // only 1/2 of amps necessary




Adafruit_NeoPixel light_bar=Adafruit_NeoPixel(NUMPIXELS, CHAIN_PIN_1, NEO_GRB + NEO_KHZ800);



void output_led_setup()
{
    light_bar.begin(); 
    light_bar.setBrightness(PIXEL_BRIGHTNESS);                                 
}

/* output_setLightColor takes RGB values, from 0,0,0 up to 255,255,255 */
void output_setLightColor(byte pixel_index,t_color_hsv color_hsv)
{
  t_color_rgb rgb = get_color_rgb(t_color_hsv);
  light_bar.setPixelColor(pixel_index, light_bar.Color(rgb.r,rgb.g,rgb.b));
}

void output_show()
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
    p = color_hsv.v * (1.0 - m_color_hsv.s); // whitening color element
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



