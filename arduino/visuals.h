#ifndef __INC_VISUALS_H
#define __INC_VISUALS_H

#define PULSE_MAX_VAL 255
#define PULSE_MIN_VAL 160
#define CHANGE_MIN_VAL 64

#define IDLE_TIMEOUT 120000 // 2 minutes

typedef enum Color {
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
long pulse_durations[5] = {800 /* used for fading up */, 6000, 3000, 0, 800};

typedef enum {
  BRIGHTNESS_MIN,
  BRIGHTNESS_MED,
  BRIGHTNESS_MAX,
  BRIGHTNESS_IDLE
} Brightness;
uint8_t brightnesses[4] = {32, 106, 255, 16};

#endif
