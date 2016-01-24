#include "FastLED.h"

// hardware
#define NUM_LEDS 12
#define LED_CORRECTION TypicalSMD5050

#define DATA_PIN 3

// visuals
#define PULSE_MAX_VAL 255
#define PULSE_MIN_VAL 160
#define PULSE_DURATION_SLOW 3000
#define PULSE_DURATION_MED 1500
#define PULSE_DURATION_FAST 0

#define CHANGE_MIN_VAL 64
#define CHANGE_DURATION 800

// state
CRGB leds[NUM_LEDS];
bool initialized;
uint8_t hue;
uint8_t val;
bool pulse;
long pulse_duration;



void fade(uint8_t new_val, long duration) {
  if (val == new_val) {
    return;
  }
  
  int incr;
  long delay_ms;
  long change_delay_ms;
  if (val < new_val) {
    incr = 1;
    delay_ms = duration / (new_val - val);
    if (CHANGE_DURATION / 2 < duration) {
      change_delay_ms = (CHANGE_DURATION / 2) / (new_val - val);
    } else {
      change_delay_ms = delay_ms;
    }
  } else if (val > new_val) {
    incr = -1;
    delay_ms = duration / (val - new_val);
    if (CHANGE_DURATION / 2 < duration) {
      change_delay_ms = (CHANGE_DURATION / 2) / (val - new_val);
    } else {
      change_delay_ms = delay_ms;
    }
  }

  for (uint8_t v = val; v != new_val; v += incr) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (initialized) {
        leds[i] = CHSV(hue, 255, v);
      } else {
        leds[i] = CRGB(v, v, v);
      }
    }
    FastLED.show();

    // check to see if we've been given a new command, if so, hurry things along
    if (Serial.available() > 0) {
      delay(change_delay_ms);
    } else {
      delay(delay_ms);
    }
  }

  val = new_val;
}

void setup() {
  Serial.begin(57600);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setCorrection(LED_CORRECTION);

  initialized = false;
  val = PULSE_MAX_VAL;
  pulse = true;
  pulse_duration = PULSE_DURATION_MED;

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(val, val, val);
  }
  FastLED.show();
}

void loop() {
  if (pulse) {
    fade(PULSE_MIN_VAL, pulse_duration / 2);
  }

  if (Serial.available() > 0) {
    byte c = Serial.read();
    c -= 65; // align to ASCII "A"
    if (c < 24) { // filter out garbage values
      fade(CHANGE_MIN_VAL, CHANGE_DURATION / 2);

      switch (c & B00000011) {
        case 0:
          pulse = false;
          pulse_duration = CHANGE_DURATION;
          break;
        case 1:
          pulse = true;
          pulse_duration = PULSE_DURATION_SLOW;
          break;
        case 2:
          pulse = true;
          pulse_duration = PULSE_DURATION_MED;
          break;
        case 3:
          pulse = true;
          pulse_duration = PULSE_DURATION_FAST;
          break;
      }
  
      switch ((c & B00011100) >> 2) {
        case 0:
          hue = HUE_RED;
          break;
        case 1:
          hue = HUE_ORANGE;
          break;
        case 2:
          hue = HUE_YELLOW;
          break;
        case 3:
          hue = HUE_GREEN;
          break;
        case 4:
          hue = HUE_BLUE;
          break;
        case 5:
          hue = HUE_PURPLE;
          break;
      }

      initialized = true;

      fade(PULSE_MIN_VAL, CHANGE_DURATION / 2);
    }
  }

  fade(PULSE_MAX_VAL, pulse_duration / 2);
}
