require 'mkmf'

unless find_library('gif', 'DGifOpen')
	abort "libgif does not include entry point DGifOpen"
end

create_makefile('composite')
