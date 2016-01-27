#include "FastLED.h"
#include "visuals.h"
#include "state.h"

// hardware
#define NUM_LEDS 12
#define LED_CORRECTION TypicalSMD5050

#define DATA_PIN 13
#define BUTTON_IN 10
#define ANALOG_INPUT A9
#define EXTRA_PIN_A 7
#define EXTRA_PIN_B 11

// state
CRGB leds[NUM_LEDS];

uint8_t val;

State current;
State next;

bool buttonDebounced;
uint8_t brightness;

unsigned long lastCommand;

void checkSerial() {
  if (Serial.available() == 0) {
    return;
  }

  uint8_t command = Serial.read();
  lastCommand = millis();

  if (command >= 60 && command < 63) { // brightness
    command -= 60; // align to ASCII "<"
    brightness = (Brightness)command;

    // handle this immediately
    FastLED.setBrightness(brightnesses[brightness]);
  } else if (command >= 65 && command < 89) { // color and pulse
    command -= 65; // align to ASCII "A"
    next.color = (Color)((command & B00011100) >> 2);
    next.pulse = (Pulse)(command & B00000011);

    // handle change later at low point in fade for a nice smooth transition
  }
}

void fade(uint8_t next_val, Pulse pulse) {
  long duration = pulse_durations[pulse] / 2;

  if (next_val == val) {
    // check for input before returning, otherwise we won't respond to changes
    // when pulse is PULSE_NONE
    checkSerial();

    // setBrightness() requires a call to delay() periodically--without this,
    // the brightness will not change when pulse is PULSE_NONE
    FastLED.delay(duration);

    return;
  }

  int8_t incr;
  if (val < next_val) {
    incr = 1;
  } else if (val > next_val) {
    incr = -1;
  }

  long delay_ms = duration / (next_val - val) * incr;
  long change_delay_ms = (pulse_durations[PULSE_CHANGE] / 2) / (next_val - val) * incr;
  if (delay_ms < change_delay_ms) {
    change_delay_ms = delay_ms;
  }

  for (uint8_t v = val; v != next_val; v += incr) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      if (current.color == COLOR_WHITE) {
        leds[i] = CRGB(v, v, v);
      } else {
        leds[i] = CHSV(colors[current.color], 255, v);
      }
    }
    FastLED.show();

    // speed up the fade if command was received and state is changing
    checkSerial();
    if (next != current) {
      FastLED.delay(change_delay_ms);
    } else {
      FastLED.delay(delay_ms);
    }
  }

  val = next_val;
}

void setup() {
  Serial.begin(57600);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setCorrection(LED_CORRECTION);

  val = PULSE_MAX_VAL;

  current.pulse = PULSE_MED;
  next = current;

  brightness = BRIGHTNESS_MED;
  FastLED.setBrightness(brightnesses[brightness]);

  lastCommand = millis();

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
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
  if (current.pulse != PULSE_NONE) {
    fade(PULSE_MIN_VAL, current.pulse);
  }

  if (next != current) {
    if (next.color != current.color) {
      // if color is changing, fade to a lower brightness first to make the change less jarring
      fade(CHANGE_MIN_VAL, PULSE_CHANGE);

      current = next;

      // wake up from idle on color change, but do it at lowest brightness
      FastLED.setBrightness(brightnesses[brightness]);

      fade(PULSE_MIN_VAL, PULSE_CHANGE);
    } else {
      current = next;

      // wake up from idle on pulse change
      FastLED.setBrightness(brightnesses[brightness]);
    }
  }

  if (millis() > lastCommand + IDLE_TIMEOUT) {
    FastLED.setBrightness(brightnesses[BRIGHTNESS_IDLE]);
  } else {
    // wake up from idle on any change, even invalid commands
    // this call is redundant in the case of state change (already woken above),
    // but setBrightness() is idempotent so it's ok
    FastLED.setBrightness(brightnesses[brightness]);
  }

  fade(PULSE_MAX_VAL, current.pulse);
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
    brightness = (brightness + 1) % 3;
    FastLED.setBrightness(brightnesses[brightness]);
  }
}
