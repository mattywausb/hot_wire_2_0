#ifndef __mainSettings_h__
#define __mainSettings_h__

#define TRACE_ON 

#define ZONE_NONE -1
#define ZONE_HOT 0
#define ZONE_LANDING_1 1
#define ZONE_LANDING_2 2

typedef struct {
    float r;       // a fraction between 0 and 1
    float g;       // a fraction between 0 and 1
    float b;       // a fraction between 0 and 1
} t_color_rgb;

typedef struct {
    float h;       // angle in degrees
    float s;       // a fraction between 0 and 1
    float v;       // a fraction between 0 and 1
} t_color_hsv;

#endif
