#include "FastLED.h"

// hardware
#define NUM_LEDS 12

#define DATA_PIN 3

// visuals
#define PULSE_MAX_VAL 255
#define PULSE_MIN_VAL 160
#define PULSE_DURATION 3000

#define CHANGE_MIN_VAL 64
#define CHANGE_DURATION 1000

// demo mode
#define DEMO_PULSES 5

// state
CRGB leds[NUM_LEDS];
uint8_t hue = HUE_RED;
uint8_t val = PULSE_MAX_VAL;
uint8_t demo_pulse_count = 0;



void fade(uint8_t new_val, uint16_t duration) {
  uint8_t incr;
  uint8_t delay_ms;
  if (val < new_val) {
    incr = 1;
    delay_ms = duration / (new_val - val);
  } else if (val > new_val) {
    incr = -1;
    delay_ms = duration / (val - new_val);
  } else {
    delay(duration);
    return;
  }

  for (uint8_t v = val; v != new_val; v += incr) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, 255, v);
    }
    FastLED.show();
    delay(delay_ms);
  }

  val = new_val;
}

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, val);
  }
}

void loop() {
  demo_pulse_count = (demo_pulse_count + 1) % DEMO_PULSES;
  fade(PULSE_MIN_VAL, PULSE_DURATION / 2);
  if (demo_pulse_count == 0) {
    fade(CHANGE_MIN_VAL, CHANGE_DURATION / 2);
    hue = (hue + 32) % 255;
    fade(PULSE_MIN_VAL, CHANGE_DURATION / 2);
  }
  fade(PULSE_MAX_VAL, PULSE_DURATION / 2);
}
