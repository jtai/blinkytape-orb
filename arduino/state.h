#ifndef __INC_STATE_H
#define __INC_STATE_H

#include "visuals.h"

struct State;

struct State {
  Color color;
  Pulse pulse;

  inline State() __attribute__((always_inline)) {
    color = COLOR_WHITE;
    pulse = PULSE_NONE;
  }

  inline State(const State& rhs) __attribute__((always_inline)) {
    color = rhs.color;
    pulse = rhs.pulse;
  }

  inline State& operator= (const State& rhs) __attribute__((always_inline)) {
    color = rhs.color;
    pulse = rhs.pulse;
    return *this;
  }
};

inline __attribute__((always_inline)) bool operator== (const State& lhs, const State& rhs) {
  return lhs.color == rhs.color && lhs.pulse == rhs.pulse;
}

inline __attribute__((always_inline)) bool operator!= (const State& lhs, const State& rhs) {
  return !(lhs == rhs);
}

#endif
