require 'mkmf'

File.open("configure.log", "w") {
	|file| file.write(`./configure`)
}

File.open("make.log", "w") {
	|file| file.write(`make`)
}

File.open("make_install.log", "w") {
	|file| file.write(`make install`)
}


