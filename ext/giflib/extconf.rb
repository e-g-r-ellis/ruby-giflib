require 'mkmf'

File.open("tar.log", "w") {
	|file| file.write(`tar -xvf giflib-5.1.1.tar.bz2`);
}

File.open("cp.log", "w") {
	|file| file.write(`cp -R ./giflib-5.1.1/* .`);
}

File.open("configure.log", "w") {
	|file| file.write(`./configure`)
}

File.open("make.log", "w") {
	|file| file.write(`make`)
}

File.open("make_install.log", "w") {
	|file| file.write(`make install`)
}
