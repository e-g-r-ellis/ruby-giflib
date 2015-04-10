#!/bin/bash

if grep -i LD_LIBRARY_PATH ~/.bash_profile | grep -i /usr/local/lib/
then
	echo "LD_LIBRARY_PATH includes /usr/local/lib/"
else
	echo "LD_LIBRARY_PATH does not include /usr/local/lib/"
	echo -e "LD_LIBRARY_PATH=/usr/local/lib/:\$LD_LIBRARY_PATH\nexport LD_LIBRARY_PATH\n" >> ~/.bash_profile
fi
