require 'minitest/autorun'
require 'giflib/composite'

class GiflibTest < Minitest::Test
	def loadImage(path)
		gif = IO.read(path)
                Composite::Image.new gif
	end

	def test_width
		image = loadImage './test/pizza/background/background.gif'
		assert(image.getWidth == 640, "Width: "+image.getWidth.to_s+" expected 640")
	end

	def test_height
		image = loadImage './test/pizza/background/background.gif'
		assert(image.getHeight == 398, "Height: "+image.getHeight.to_s+" expected 398")
	end

	def test_savedImage
		image = loadImage './test/pizza/background/background.gif'
		assert(image.getImageCount == 1, "ImageCount: "+image.getImageCount.to_s+" expected 1")
	end

	def test_encode
		image = loadImage './test/pizza/background/background.gif'
		encoded = image.encode
		assert(encoded.length == 152945, "Encoded length: "+encoded.length.to_s+" expected 152945")
	end

	def test_composite
		expected = loadImage './test/pizza/composed.gif'
		image = loadImage './test/pizza/background/background.gif'
		day = loadImage './test/pizza/days/days-01.gif'
		image.compose day, 41, 205
		encoded = image.encode
		IO.write('./composed.gif', encoded)
		assert(encoded == expected, "")
	end
end
