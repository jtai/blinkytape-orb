BlinkyTape Orb
==============

Ruby code to interface with a [BlinkyTape controller board](http://blinkinlabs.myshopify.com/collections/frontpage/products/blinkytape-control-board)
running the custom firmware in the `arduino` directory of this repository. See
the [Blinkinlabs arduino instructions](http://blinkinlabs.com/blinkytape/arduino/)
for how to install the firmware. The firmware can be made to work on an Arduino
Uno R3 with some modifications, mainly around the button handling.

```ruby
require 'blinkytape-orb'

# device is automatically detected on OS X
orb = BlinkyTapeOrb.new

# a device can be passed manually
#orb = BlinkyTapeOrb.new(:device => '/dev/ttyS0')

# a logger can also be passed
#orb = BlinkyTapeOrb.new(:logger => Logger.new(STDOUT))
#orb.logger.level = Logger::DEBUG

[
  [ BlinkyTapeOrb::COLOR_GREEN,  BlinkyTapeOrb::PULSE_NONE ],
  [ BlinkyTapeOrb::COLOR_BLUE,   BlinkyTapeOrb::PULSE_SLOW ],
  [ BlinkyTapeOrb::COLOR_PURPLE, BlinkyTapeOrb::PULSE_SLOW ],
  [ BlinkyTapeOrb::COLOR_YELLOW, BlinkyTapeOrb::PULSE_MED  ],
  [ BlinkyTapeOrb::COLOR_ORANGE, BlinkyTapeOrb::PULSE_MED  ],
  [ BlinkyTapeOrb::COLOR_RED,    BlinkyTapeOrb::PULSE_FAST ],
].each do |(color, animation)|
  orb.update(color, animation)
  sleep(10)
end
```
