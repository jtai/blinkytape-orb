require 'blinkytape-orb/version'
require 'logger'
require 'serialport'

class BlinkyTapeOrb

  SERIAL_PORT_BAUD_RATE = 57600

  COLOR_RED    = 0
  COLOR_ORANGE = 1
  COLOR_YELLOW = 2
  COLOR_GREEN  = 3
  COLOR_BLUE   = 4
  COLOR_PURPLE = 5

  PULSE_NONE = 0
  PULSE_SLOW = 1
  PULSE_MED  = 2
  PULSE_FAST = 3

  BRIGHTNESS_MIN = 0
  BRIGHTNESS_MED = 1
  BRIGHTNESS_MAX = 2

  def initialize(options = {})
    @opts = options
  end

  def device
    @device ||= begin
      dev = @opts[:device] || autodetect_device

      unless dev && File.exist?(dev) && File.stat(dev).chardev?
        raise ArgumentError, 'invalid device'
      end

      logger.info("selected device #{dev}")

      dev
    end
  end

  def logger
    @logger ||= begin
      @opts[:logger] || Logger.new(nil)
    end
  end

  def setColorAndPulse(color, pulse)
    if (color < 0 || color > 5)
      raise ArgumentError, 'color must be between 0 and 5'
    end

    if (pulse < 0 || pulse > 3)
      raise ArgumentError, 'pulse must be between 0 and 3'
    end

    logger.info("updating with color=#{color}, pulse=#{pulse}")

    send(('A'.ord + (color * 4) + pulse).chr)
  end

  def setBrightness(brightness)
    if (brightness < 0 || brightness > 2)
      raise ArgumentError, 'brightness must be between 0 and 2'
    end

    logger.info("setting brightness=#{brightness}")

    send(('<'.ord + brightness).chr)
  end

  private

  def autodetect_device
    devices = Dir.glob('/dev/*.usbmodem*') + Dir.glob('/dev/*.usbserial*')

    devices.each do |dev|
      logger.debug("autodetect_device found #{dev}")
    end

    devices.first
  end

  def send(command)
    SerialPort.open(device, SERIAL_PORT_BAUD_RATE) do |serial_port|
      logger.debug("send \"#{command}\"")
      serial_port.write(command)
    end
  end

end
