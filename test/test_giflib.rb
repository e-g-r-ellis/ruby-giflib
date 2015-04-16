require 'minitest/autorun'
require 'giflib/composite'

class GiflibTest < Minitest::Test
	def loadImage
		gif = IO.read('./test/pizza/background/background.gif')
		puts gif.slice(0,10)
                Composite::Image.new gif
	end

	#def test_when_new_image_created_doesNotCrash
	#	loadImage
	#end

	#def test_width
	#	image = loadImage
	#	assert(image.getWidth == 640, "Width: "+image.getWidth.to_s+" expected 640")
	#end

	#def test_height
	#	image = loadImage
	#	assert(image.getHeight == 398, "Height: "+image.getHeight.to_s+" expected 398")
	#end

	#def test_savedImage
	#	image = loadImage
	#	assert(image.getImageCount == 1, "ImageCount: "+image.getImageCount.to_s+" expected 1")
	#end

	def test_encode
		image = loadImage
		encoded = image.encode
	end
end
