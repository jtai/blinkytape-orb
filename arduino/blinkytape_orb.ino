#include "FastLED.h"

/* hardware */
#define NUM_LEDS 12

#define DATA_PIN 3

/* visuals */
#define PULSE_MAX_VAL 255
#define PULSE_MIN_VAL 160
#define PULSE_CYCLE 2000
#define PULSE_DELAY PULSE_CYCLE / (PULSE_MAX_VAL - PULSE_MIN_VAL)

#define CHANGE_MIN_VAL 64
#define CHANGE_CYCLE 500
#define CHANGE_DELAY CHANGE_CYCLE / (PULSE_MIN_VAL - CHANGE_MIN_VAL)

/* demo mode */
#define DEMO_PULSES 5

/* state */
CRGB leds[NUM_LEDS];
uint8_t hue = HUE_RED;
uint8_t demo_pulse_count = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, PULSE_MAX_VAL);
  }
}

void loop() {
  demo_pulse_count = (demo_pulse_count + 1) % DEMO_PULSES;

  for (uint8_t v = PULSE_MAX_VAL; v > PULSE_MIN_VAL; v--) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, 255, v);
    }
    FastLED.show();
    delay(PULSE_DELAY);
  }

  /* demo mode */
  if (demo_pulse_count == 0) {
    for (uint8_t v = PULSE_MIN_VAL; v > CHANGE_MIN_VAL; v--) {
      for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue, 255, v);
      }
      FastLED.show();
      delay(CHANGE_DELAY);
    }

    hue = (hue + 32) % 255;

    for (uint8_t v = CHANGE_MIN_VAL; v < PULSE_MIN_VAL; v++) {
      for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue, 255, v);
      }
      FastLED.show();
      delay(CHANGE_DELAY);
    }
  }

  for (uint8_t v = PULSE_MIN_VAL; v < PULSE_MAX_VAL; v++) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, 255, v);
    }
    FastLED.show();
    delay(PULSE_DELAY);
  }
}
