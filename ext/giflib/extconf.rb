require 'mkmf'

unless find_library('gif', 'DGifOpen')
	abort "libgif does not include entry point DGifOpen"
else
	puts "Library 'gif' does have entry point 'DGifOpen'\n"
end

create_makefile('composite')
