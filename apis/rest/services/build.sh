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
	echo "Building PKDuck ..."
	cd $pwd/pkduck_service/code/
	rm makefile
	cmake .
	make
	echo "... Done!"
)

(
	echo "Building ImputeDB ..."
	cd $pwd/imputedb_service/imputedb/
	export JAVA_TOOL_OPTIONS=-Dfile.encoding=UTF8
	ant
	# ant -Dfile.encoding=UTF8
	echo "... Done!"
)

#GloVe embeddings for DeepER
(
cd $pwd/deeper_service/DeepER-Lite/glove/
glove=glove.840B.300d.zip
wget http://nlp.stanford.edu/data/$glove
unzip $glove
rm -f $glove
)


# (
# 	echo "Building Golden Record ..."
# 	cd $pwd/grecord_service/code/
# 	python test-so.py
# 	ls -trlh
# 	echo "... Done!"
# )
