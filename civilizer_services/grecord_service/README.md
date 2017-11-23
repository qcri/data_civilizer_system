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

### Run Golden Record

First import call_goldenrecord and then call the main function in call_goldenrecord. See the jupyter notebook file for an example

### Interaction

It will go throught each column in the input table and ask if you want to run golden record on the column. You will have 5 options:

0 (default): skip,  

1: replacing rule only, 

2: replacing rules + full rules, 

3: replacing rules + deletion rules, 

4: replacing rules + deletion rules + full rules,

5: full rules only

For shorter values (e.g., names) use 2 or 4. For longer values (e.g., full address) use 1 or 3. Note that:

* Replacing rules are the rules like 9 <-> 9th, which is a pair of substrings in a pair of value within the same cluster.

* Deletion rules are the rules with one side as empty.

* Full rules uses a pair of values within the same cluster as a single rule.




