require 'mkmf'

# Extract giflib
File.open("tar.log", "w") {
	|file| file.write(`tar -xvf giflib-5.1.1.tar.bz2`)
}

# Build giflib in extracted directory (build tool appears not to do this for directories created from within the build hook)
Dir.chdir("./giflib-5.1.1")

File.open("configure.log", "w") {
	|file| file.write(`./configure`)
}

File.open("make.log", "w") {
	|file| file.write(`make`)
}

File.open("make_install.log", "w") {
	|file| file.write(`sudo make install`)
}

Dir.chdir("..")

FileUtils.touch('./Makefile')

