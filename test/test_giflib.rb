require 'minitest/autorun'
require 'giflib/composite'

class GiflibTest < Minitest::Test
	def test_when_new_image_created
		gif = IO.read('./test/pizza/background/background.gif')
		STDERR.puts " gif: "+gif.slice(0,30)+"\n"
		Composite::Image.new gif
	end
end
