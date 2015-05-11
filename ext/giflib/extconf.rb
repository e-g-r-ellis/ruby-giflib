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

# Determine OS version (mac/EC2) for Makefile
os=`uname`
case os
when "Linux
"
	fname="ec2_Makefile"
when "Darwin
"
	fname="mac_Makefile"
end
puts "Running on (#{fname}) (#{os})"

command="cp #{fname} Makefile"
File.open("cp.log","w") {
	|file|
	file.write(command)
	IO.popen(command) {
		|p| puts p.gets
	}
}

# Update LD_LIBRARY_PATH
File.open("chmod.log","w") {
	|file| file.write(`chmod u+x libraryPath.sh`)
}

File.open("libraryPath.log","w") {
	|file| file.write(`./libraryPath.sh`)
}

