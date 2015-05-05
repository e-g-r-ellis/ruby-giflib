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

	def test_extensionBlocks
		image = loadImage './test/pizza/background/background.gif'
		assert(image.getSavedImageExtensionBlockCount(0) == 1, "Expected 1 (why?) but got "+image.getSavedImageExtensionBlockCount(0).to_s)
	end

	def test_extensionBlocksAfterSetDelay
		image = loadImage './test/pizza/background/background.gif'
		image.setDelayTimeForFrame(0,2)
		assert(image.getDelayTimeForFrame(0) == 2, "Expected 2 but got "+image.getDelayTimeForFrame(0).to_s)
		assert(image.getSavedImageExtensionBlockCount(0) == 1, "Expected 1 but got "+image.getSavedImageExtensionBlockCount(0).to_s)
	end

	def test_addFrame
		image = loadImage './test/pizza/background/background.gif'
		image.addFrame image
		assert(image.getImageCount == 2, "ImageCount: "+image.getImageCount.to_s+" expected 2")
	end

	def test_encode
		image = loadImage './test/pizza/background/background.gif'
		encoded = image.encode
		assert(encoded.length == 152945, "Encoded length: "+encoded.length.to_s+" expected 152945")
	end

	def test_setDelay
		image = loadImage './test/pizza/background/background.gif'

		image.setDelayTimeForFrame(0,10)
		actual = image.getDelayTimeForFrame(0)
		assert(actual == 10, "Expected frame delay time to be 10 but was actually "+actual.to_s)

		image.setDelayTimeForFrame(0,20)
		actual = image.getDelayTimeForFrame(0)
		assert(actual == 20, "Expected frame delay time to be 20 but was actually "+actual.to_s)

		encoded = image.encode
		IO.write('./setDelay.gif', encoded)
		image2 = Composite::Image.new encoded
	end

	def test_composite
		image = loadImage './test/pizza/background/background.gif'
		day = loadImage './test/pizza/days/days-01.gif'
		hour = loadImage './test/pizza/hours/pec-hours-00.gif'
		minutes = loadImage './test/pizza/minutes/pec-minutes-01.gif'
		seconds = loadImage './test/pizza/seconds/pec-seconds-11.gif'

		image.compose day, 41, 205
		image.compose hour, 186, 205
		image.compose minutes, 331, 205
		image.compose seconds, 476, 205
		encoded = image.encode
		IO.write('./composed.gif', encoded)
		#assert(encoded == expected, "")
	end
end
