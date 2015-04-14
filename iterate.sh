#!/bin/bash
rm giflib*.gem
gem uninstall giflib
gem build ./giflib.gemspec.rb
gem install giflib-*.gem 2>&1 | tee error.log
if [ $? == 0 ]
then
	rake 2>&1 | tee test.log
fi
