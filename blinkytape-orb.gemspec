# encoding: utf-8
$:.push File.expand_path("../lib", __FILE__)

require 'blinkytape-orb/version'

Gem::Specification.new do |s|
  s.name         = "blinkytape-orb"
  s.version      = BlinkyTapeOrb::VERSION
  s.platform     = Gem::Platform::RUBY
  s.authors      = ["Jon Tai"]
  s.email        = ["jon.tai@gmail.com"]
  s.homepage     = "https://github.com/jtai/blinkytape-orb"
  s.summary      = "Ruby code to interface with BlinkyTape controller board running custom firmware"
  s.description  = "Other hardware (e.g., arduino) may be supported with minor firmware modifications"

  s.files        = `git ls-files`.split("\n")
  s.test_files   = `git ls-files -- spec/*`.split("\n")
  s.require_path = 'lib'

  s.add_runtime_dependency 'serialport', '>= 1.3.1'
end
