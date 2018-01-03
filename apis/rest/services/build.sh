#!/bin/bash
set -e 
pwd=`dirname $0`

(
	echo "Building Fahes ..."
	cd $pwd/fahes_service/fahes/
	make
	echo "... Done!"
)

(
	echo "Building Golden Record ..."
	cd $pwd/grecord_service/code/
	make
	echo "... Done!"
)

(
	echo "Building PKDuck ..."
	cd $pwd/pkduck_service/code/
	make
	echo "... Done!"
)
