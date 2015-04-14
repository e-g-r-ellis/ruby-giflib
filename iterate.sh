#!/bin/bash
rm giflib*.gem
gem uninstall giflib
gem build ./giflib.gemspec.rb
gem install giflib-*.gem
if [ $? == 0 ]
then
	rake
fi
