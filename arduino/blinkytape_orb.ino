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
#define PULSE_DURATION_SLOW 6000
#define PULSE_DURATION_MED 3000
#define PULSE_DURATION_FAST 0

#define CHANGE_MIN_VAL 64
#define CHANGE_DURATION 800

#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_MED 106
#define BRIGHTNESS_MIN 32

// state
CRGB leds[NUM_LEDS];

byte prev_command;
byte command;
bool initialized;

uint8_t hue;
uint8_t val;
bool pulse;
long pulse_duration;

bool buttonDebounced;
uint8_t brightness;


void checkSerial() {
  if (Serial.available() > 0) {
    command = Serial.read();
  }

  // handle certain commands immediately, then swallow the
  // handled commands to prevent rushing through the next fade
  if (command >= 60 && command < 63) { // brightness
    switch (command - 60) { // align to ASCII "<"
      case 0:
        brightness = BRIGHTNESS_MIN;
        break;
      case 1:
        brightness = BRIGHTNESS_MED;
        break;
      case 2:
        brightness = BRIGHTNESS_MAX;
        break;
    }

    FastLED.setBrightness(brightness);
    command = prev_command;
  } else if (command >= 65 && command < 89) { // color and pulse
    // handled later at low point in fade for a nice smooth transition
  } else { // invalid command
    command = prev_command;
  }
}

void fade(uint8_t new_val, long duration) {
  if (val == new_val) {
    checkSerial();

    // setBrightness() requires a call to delay() periodically
    // without this line, we can't change the brightness when we're not pulsing
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

    // check to see if we've been given a new hue or pulse command
    // if so, hurry things along
    checkSerial();
    if (command != prev_command) {
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

  prev_command = 122; // initialize to invalid command so any valid command will trigger action
  command = 122;
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

  if (command != prev_command) {
    fade(CHANGE_MIN_VAL, CHANGE_DURATION / 2);

    prev_command = command;
    initialized = true;

    byte c = command - 65; // align to ASCII "A"

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

    fade(PULSE_MIN_VAL, CHANGE_DURATION / 2);
  }

  fade(PULSE_MAX_VAL, pulse_duration / 2);
}

// Called when the button is both pressed and released
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

// Called every xx ms while the button is being held down
ISR(TIMER4_OVF_vect) {
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
