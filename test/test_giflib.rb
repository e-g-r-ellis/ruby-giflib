require 'minitest/autorun'
require 'giflib/composite'

class GiflibTest < Minitest::Test
	def test_when_new_image_created_doesNotCrash
		gif = IO.read('./test/pizza/background/background.gif')
		Composite::Image.new gif
		assert true
	end
end
