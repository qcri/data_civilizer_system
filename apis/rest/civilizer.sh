#!/bin/bash

#demo-g#
# cd ./rest/services/deeper_service/DeepER-Lite/glove/
# if [ ! -f GloVe.t7 ]
# then
#     echo GloVe word vector file not found.  Initializing...
#     glove=glove.840B.300d.zip
#     echo Step 1/5 - Downloading $glove...
#     wget http://nlp.stanford.edu/data/$glove
#     echo Step 2/5 - Unzipping $glove...
#     unzip $glove > /dev/null
#     echo Step 3/5 - Deleting $glove...
#     rm -f $glove
#     sed "/outfilename =/ s/'[^']*'/'\/app\/storage\/glove\/GloVe.t7'/" -i glove_setup.lua
#     [ ! -d /app/storage/glove ] && mkdir /app/storage/glove
#     echo Step 4/5 - Converting GloVe words to vectors...
#     th glove_setup.lua > /dev/null
#     echo Step 5/5 - Deleting ${glove/zip/txt}...
#     rm -f ${glove/zip/txt}
#     echo GloVe initialization completing.
# fi
cd /app/

python ./rest/civilizer.py
