PKDuck -- Approximate String Joins with Abbreviations
===================

Introduction
----------
PKDuck is a data cleaning tool capable of finding similar strings in several given columns. In particular, PKDuck handles abbreviations well. For example, the following are several pairs of strings with abbreviations that can be identified by PKDuck. 

String 1     | String 2
-------- | -------
CSAIL hd qtr  | CSAIL head quarter
schl of management    | school of mgmt
intro to how to make peking duck     |  introduction to how to make PKDuck

PKDuck uses a novel string similarity measure that utilizes an abbreviation dictionary. This measure can not only find similar strings identifable by traditional measures (e.g. Edit Distance), it also captures abbreviations well and thus has much better recall than traditional measures. PKDuck also uses efficient signature-based join algorithms that make it scale to very large datasets. 

For more details on the internals of PKDuck, see our research paper [here](http://www.vldb.org/pvldb/vol11/p53-tao.pdf). 

How to Run PKDuck
-------------
