#!/bin/bash
set -e 
pwd=`dirname $0`

(
	echo "Building Golden Record ..."
	cd $pwd/code/
	python --version
	# make clean
	# make
	# python test-so.py
	python setup.py build_ext --inplace 
	ls -trlh
	echo "... Done!"
)


