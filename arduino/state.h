struct State;

struct State {
  uint8_t hue;
  bool pulse;
  long pulse_duration;

  inline State() __attribute__((always_inline)) {
  }

  inline State(uint8_t ihue, bool ipulse, long ipulse_duration) __attribute__((always_inline))
    : hue(ihue), pulse(ipulse), pulse_duration(ipulse_duration) {
  }

  inline State(const State& rhs) __attribute__((always_inline)) {
    hue = rhs.hue;
    pulse = rhs.pulse;
    pulse_duration = rhs.pulse_duration;
  }

  inline State& operator= (const State& rhs) __attribute__((always_inline)) {
    hue = rhs.hue;
    pulse = rhs.pulse;
    pulse_duration = rhs.pulse_duration;
    return *this;
  }
};

inline __attribute__((always_inline)) bool operator== (const State& lhs, const State& rhs) {
    return (lhs.hue == rhs.hue) && (lhs.pulse == rhs.pulse) && (lhs.pulse_duration == rhs.pulse_duration);
}

inline __attribute__((always_inline)) bool operator!= (const State& lhs, const State& rhs) {
    return !(lhs == rhs);
}
