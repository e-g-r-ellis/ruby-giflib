#!/bin/bash
gem uninstall giflib
rm giflib*.gem
gem build giflib.gemspec.rb
gem install giflib*.gem

