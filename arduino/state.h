struct State;

struct State {
  bool hue_initialized;
  uint8_t hue;

  bool pulse;
  long pulse_duration;

  inline State() __attribute__((always_inline)) {
    hue_initialized = false;
  }

  inline State(const State& rhs) __attribute__((always_inline)) {
    hue_initialized = rhs.hue_initialized;
    hue = rhs.hue;
    pulse = rhs.pulse;
    pulse_duration = rhs.pulse_duration;
  }

  inline State& operator= (const State& rhs) __attribute__((always_inline)) {
    hue_initialized = rhs.hue_initialized;
    hue = rhs.hue;
    pulse = rhs.pulse;
    pulse_duration = rhs.pulse_duration;
    return *this;
  }
};

inline __attribute__((always_inline)) bool operator== (const State& lhs, const State& rhs) {
    return (lhs.hue_initialized == rhs.hue_initialized) && (lhs.hue == rhs.hue) && 
           (lhs.pulse == rhs.pulse) && (lhs.pulse_duration == rhs.pulse_duration);
}

inline __attribute__((always_inline)) bool operator!= (const State& lhs, const State& rhs) {
    return !(lhs == rhs);
}
