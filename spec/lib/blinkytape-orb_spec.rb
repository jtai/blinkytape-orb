require 'spec_helper'

describe BlinkyTapeOrb do

  let (:options) { {} }
  let (:orb) { BlinkyTapeOrb.new(options) }

  describe '#device' do

    context 'with valid device option' do
      let (:options) { { :device => '/dev/zero' } }

      it 'returns device' do
        expect(orb.device).to eq('/dev/zero')
      end
    end

    context 'with non-existent device' do
      let (:options) { { :device => '/dev/nosuchdevice' } }

      it 'raises ArgumentError' do
        expect {
          orb.device
        }.to raise_error(ArgumentError)
      end
    end

    context 'with block device' do
      let (:options) { { :device => `df #{__FILE__} | tail -1 | cut -d ' ' -f 1` } }

      it 'raises ArgumentError' do
        expect {
          orb.device
        }.to raise_error(ArgumentError)
      end
    end

    context 'with autodetected device' do
      it 'returns first device matching pattern' do
        expect(Dir).to receive(:glob).twice.and_return(['/dev/urandom', '/dev/random'])
        expect(orb.device).to eq('/dev/urandom')
      end
    end

  end

  describe '#setColorAndPulse' do

    let (:color) { BlinkyTapeOrb::COLOR_RED }
    let (:pulse) { BlinkyTapeOrb::PULSE_NONE }

    context 'with out of range color' do
      let (:color) { 6 }

      it 'raises ArgumentError' do
        expect {
          orb.setColorAndPulse(color, pulse)
        }.to raise_error(ArgumentError)
      end
    end

    context 'with out of range pulse' do
      let (:pulse) { 4 }

      it 'raises ArgumentError' do
        expect {
          orb.setColorAndPulse(color, pulse)
        }.to raise_error(ArgumentError)
      end
    end

    context 'with valid arguments' do
      it 'sends correct command' do
        expect(orb).to receive(:send).with('A')
        orb.setColorAndPulse(color, pulse)
      end
    end

    context 'with purple color' do
      let (:color) { BlinkyTapeOrb::COLOR_PURPLE }

      it 'sends correct command' do
        expect(orb).to receive(:send).with('U')
        orb.setColorAndPulse(color, pulse)
      end
    end

    context 'with fast pulse' do
      let (:pulse) { BlinkyTapeOrb::PULSE_FAST }

      it 'sends correct command' do
        expect(orb).to receive(:send).with('D')
        orb.setColorAndPulse(color, pulse)
      end
    end

  end

  describe '#setBrightness' do

    let (:brightness) { BlinkyTapeOrb::BRIGHTNESS_MAX }

    context 'with out of range brightness' do
      let (:brightness) { 3 }

      it 'raises ArgumentError' do
        expect {
          orb.setBrightness(brightness)
        }.to raise_error(ArgumentError)
      end
    end

    context 'with valid arguments' do
      it 'sends correct update' do
        expect(orb).to receive(:send).with('c')
        orb.setBrightness(brightness)
      end
    end

    context 'with min brightness' do
      let (:brightness) { BlinkyTapeOrb::BRIGHTNESS_MIN }

      it 'sends correct update' do
        expect(orb).to receive(:send).with('a')
        orb.setBrightness(brightness)
      end
    end

  end

  describe '#send' do

    let (:options) { { :device => '/dev/zero' } }
    let (:io) { IO.new }

    it 'sends commands' do
      io = double('serial_port')
      expect(io).to receive(:write).with('A')
      expect(SerialPort).to receive(:open).and_yield(io)
      orb.setColorAndPulse(BlinkyTapeOrb::COLOR_RED, BlinkyTapeOrb::PULSE_NONE)
    end

  end

end
