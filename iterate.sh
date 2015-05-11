#!/bin/bash
chmod uga+rw *.gif
rm *.gif
rm giflib*.gem
gem uninstall giflib
loc="$(gem environment | grep -i installation | tr -s ' \t' | cut -d ' ' -f 5)gems/giflib-*"
echo "Deleting gem installation directory (curious that gem uninstall does not do this: $loc)"
sudo rm -rf $loc
echo "Building"
gem build ./giflib.gemspec.rb
echo "Installing"
gem install giflib-*.gem 2>&1 | tee error.log
if [ $? == 0 ]
then
	rake 2>&1 | tee test.log
fi
