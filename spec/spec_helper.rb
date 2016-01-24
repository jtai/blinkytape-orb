require 'simplecov'

SimpleCov.start do
  add_filter 'spec'
end

SimpleCov.use_merging false

require 'blinkytape-orb'

RSpec.configure do |config|
  config.color = true
end
