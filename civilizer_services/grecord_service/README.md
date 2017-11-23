Golden Record 
===================

Introduction
----------
Golden Record is an entity consolidation tool, which, given a collection of clusters of duplicate records, constructs a single record for each cluster that contains the canonical value for each attribute. 

Golden Record is written in C++ with a python wrapper.

### Compile
Go to the 'code' folder and execute the following command:

* ./run.sh 

It will produce a file '_goldenrecord.so'

### Specify Parameters
Golden Record only takes two parameters

* input_file: this parameter is a path to a CSV table that contains a set of records. The table is assumed to have an attribute with the name exactly 'cluster_id'. Records with the same value in this attribute are considered  within the same cluster.

* output_file: this parameter is a path to a file where the golden records will write into.


### Interaction
