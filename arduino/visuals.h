#ifndef __INC_VISUALS_H
#define __INC_VISUALS_H

#define VAL_MAX 255
#define VAL_MED 160
#define VAL_MIN 64

#define IDLE_TIMEOUT 120000 // 2 minutes

typedef enum {
  COLOR_RED,
  COLOR_ORANGE,
  COLOR_YELLOW,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_WHITE
} Color;
uint8_t colors[6] = {HUE_RED, HUE_ORANGE, HUE_YELLOW, HUE_GREEN, HUE_BLUE, HUE_PURPLE};

typedef enum {
  PULSE_NONE,
  PULSE_SLOW,
  PULSE_MED,
  PULSE_FAST,
  PULSE_CHANGE,
} Pulse;
long delays[5] = {3 /* used for fading up */, 30, 15, 3, 3 /* should be lower than all other delays */};

typedef enum {
  BRIGHTNESS_MIN,
  BRIGHTNESS_MED,
  BRIGHTNESS_MAX,
  BRIGHTNESS_IDLE
} Brightness;
uint8_t brightnesses[4] = {32, 144, 255, 16};

#endif
