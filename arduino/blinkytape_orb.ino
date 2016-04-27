#include <EEPROM.h>
#include <FastLED.h>

#include "visuals.h"
#include "state.h"

// hardware
#define NUM_LEDS 12
#define LED_CORRECTION TypicalSMD5050
#define COLOR_TEMPERATURE OvercastSky

#define DATA_PIN 13
#define BUTTON_IN 10
#define ANALOG_INPUT A9
#define EXTRA_PIN_A 7
#define EXTRA_PIN_B 11

#define EEPROM_START_ADDRESS 0
#define EEPROM_MAGIG_BYTE_0 0x42
#define EEPROM_MAGIC_BYTE_1 0x1e

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

long adjustedDelay(long duration, uint8_t frames) {
  long dataTransferDelay = 0.03 * NUM_LEDS * frames; // 30us per pixel
  return (duration - dataTransferDelay) / frames;
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

  int8_t incr = (val < next_val) ? 1 : -1;
  uint8_t frames = (next_val - val) * incr;
  long delay = adjustedDelay(duration, frames);
  long change_delay = adjustedDelay(pulse_durations[PULSE_CHANGE] / 2, frames);
  if (delay < change_delay) {
    change_delay = delay;
  }

  for (uint8_t v = val; v != next_val; v += incr) {
    CRGB color;
    if (current.color == COLOR_WHITE) {
      color = CRGB(v, v, v);
    } else {
      color = CHSV(colors[current.color], 255, v);
    }

    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = color;
    }

    // speed up the fade if command was received and state is changing
    checkSerial();

    if (next != current) {
      FastLED.delay(change_delay);
    } else {
      FastLED.delay(delay);
    }
  }

  val = next_val;
}

void setup() {
  Serial.begin(57600);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  val = VAL_MAX;

  current.pulse = PULSE_MED;
  next = current;

  brightness = BRIGHTNESS_MED;

  // Attempt to read in the last saved brightness
  if (EEPROM.read(EEPROM_START_ADDRESS) == EEPROM_MAGIG_BYTE_0 &&
      EEPROM.read(EEPROM_START_ADDRESS + 1) == EEPROM_MAGIC_BYTE_1) {
    uint8_t saved_brightness = EEPROM.read(EEPROM_START_ADDRESS + 2);
    if (saved_brightness < 3) {
      brightness = saved_brightness;
    }
  } else {
    EEPROM.write(EEPROM_START_ADDRESS, EEPROM_MAGIG_BYTE_0);
    EEPROM.write(EEPROM_START_ADDRESS + 1, EEPROM_MAGIC_BYTE_1);
    EEPROM.write(EEPROM_START_ADDRESS + 2, brightness);
  }

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
    fade(VAL_MED, current.pulse);
  }

  if (next.color != current.color) {
    // if color is changing, fade to a lower brightness first to make the change less jarring
    fade(VAL_MIN, PULSE_CHANGE);

    // apply color correction
    FastLED.setCorrection(LED_CORRECTION);
    FastLED.setTemperature(COLOR_TEMPERATURE);
  }

  current = next;

  if (millis() - lastCommand > IDLE_TIMEOUT) {
    FastLED.setBrightness(brightnesses[BRIGHTNESS_IDLE]);
  } else {
    // wake up from idle on color change, but do it at lowest brightness
    FastLED.setBrightness(brightnesses[brightness]);
  }

  if (val == VAL_MIN) {
    fade(VAL_MED, PULSE_CHANGE);
  }

  fade(VAL_MAX, current.pulse);
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
    EEPROM.write(EEPROM_START_ADDRESS + 2, brightness);
  }
}
