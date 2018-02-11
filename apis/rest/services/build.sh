#!/bin/bash
set -e 
pwd=`dirname $0`

(
	echo "Building Golden Record ..."
	cd $pwd/grecord_service/code/
	python test-so.py
	ls -trlh
	echo "... Done!"
)

(
	echo "Building Fahes ..."
	cd $pwd/fahes_service/fahes/
	make
	echo "... Done!"
)

(
	echo "Building PKDuck ..."
	cd $pwd/pkduck_service/code/
	make
	echo "... Done!"
)

(
	echo "Building ImputeDB ..."
	cd $pwd/imputedb_service/imputedb/
	ant -Dfile.encoding=UTF8
	echo "... Done!"
)
