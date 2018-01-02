#!/bin/bash
set -e
(
cd /app/rest/services/deeper_service/DeepER-Lite/torch
./install-deps
./install.sh
. /app/torch/install/bin/torch-activate
luarocks install csvigo
luarocks install dp
)

#GloVe embeddings
(
cd /app/rest/services/deeper_service/DeepER-Lite/glove
glove=glove.840B.300d.zip
wget http://nlp.stanford.edu/data/$glove
unzip $glove
rm -f $glove
)
