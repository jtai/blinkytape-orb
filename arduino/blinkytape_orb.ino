#include "FastLED.h"

// hardware
#define NUM_LEDS 12
#define LED_CORRECTION TypicalSMD5050

#define DATA_PIN 13
#define BUTTON_IN 10
#define ANALOG_INPUT A9
#define EXTRA_PIN_A 7
#define EXTRA_PIN_B 11

// visuals
#define PULSE_MAX_VAL 255
#define PULSE_MIN_VAL 160
#define PULSE_DURATION_SLOW 3000
#define PULSE_DURATION_MED 1500
#define PULSE_DURATION_FAST 0

#define CHANGE_MIN_VAL 64
#define CHANGE_DURATION 800

#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_MED 106
#define BRIGHTNESS_MIN 32

// state
CRGB leds[NUM_LEDS];

bool initialized;

uint8_t hue;
uint8_t val;
bool pulse;
long pulse_duration;

volatile bool buttonDebounced;
volatile uint8_t brightness;



void fade(uint8_t new_val, long duration) {
  if (val == new_val) {
    // setBrightness() requires a call to delay() periodically
    FastLED.delay(duration);
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
      FastLED.delay(change_delay_ms);
    } else {
      FastLED.delay(delay_ms);
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

  brightness = BRIGHTNESS_MED;
  FastLED.setBrightness(brightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(val, val, val);
  }
  FastLED.show();

  pinMode(BUTTON_IN, INPUT_PULLUP);
  pinMode(ANALOG_INPUT, INPUT_PULLUP);
  pinMode(EXTRA_PIN_A, INPUT_PULLUP);
  pinMode(EXTRA_PIN_B, INPUT_PULLUP);

  // Interrupt set-up; see Atmega32u4 datasheet section 11
  PCIFR  |= (1 << PCIF0);  // Just in case, clear interrupt flag
  PCMSK0 |= (1 << PCINT6); // Set interrupt mask to the button pin (PCINT6)
  PCICR  |= (1 << PCIE0);  // Enable interrupt
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

// Called when the button is both pressed and released.
ISR(PCINT0_vect){
  if (!(PINB & (1 << PINB6))) {
    buttonDebounced = false;

    // Configure and start timer4 interrupt.
    TCCR4B = 0x0F; // Slowest prescaler
    TCCR4D = _BV(WGM41) | _BV(WGM40);  // Fast PWM mode
    OCR4C = 0x10;        // some random percentage of the clock
    TCNT4 = 0;  // Reset the counter
    TIMSK4 = _BV(TOV4);  // turn on the interrupt
  } else {
    TIMSK4 = 0;  // turn off the interrupt
  }
}

// This is called every xx ms while the button is being held down; it counts down then displays a
// visual cue and changes the pattern.
ISR(TIMER4_OVF_vect) {
  // If the user is still holding down the button after the first cycle, they were serious about it.
  if (buttonDebounced == false) {
    buttonDebounced = true;

    switch (brightness) {
      case BRIGHTNESS_MIN:
        brightness = BRIGHTNESS_MED;
        break;
      case BRIGHTNESS_MED:
        brightness = BRIGHTNESS_MAX;
        break;
      case BRIGHTNESS_MAX:
        brightness = BRIGHTNESS_MIN;
        break;
    }
    FastLED.setBrightness(brightness);
  }
}
