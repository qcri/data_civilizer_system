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

How to Run PKDuck as a Standalone Program
-------------
`pkduck_api.py` is the python file that executes PKDuck. The steps needed to run PKDuck as an independent module are: 1) compile the PKDuck C++ code into a shared library that can be accessed by python, 2) specify input data and output folders and 3) specify some parameters that would be passed into the `execute_pkduck` function in `pkduck_api.py`. 

### Compile
`code/CMakeLists.txt` is the input to `cmake` to build the PKDuck shared library. To build, run the following two commands under the `code` folder:
```
$ cmake .
$ make
```
After running these two commands, a shared library file will be generated in the `code` folder. The name of the library should be `libpkduck`. Depending on the operating system, the suffix should be either `dylib` (macOS) or `so` (linux). 

### Specify Input and Output Folders
The input folder should contain all and __**only**__ input tables (in csv). The input folder name should be specified in the `input.json` file (the `["CSV"]["dir"]` field). 

The output folder name should be specified in the `output.json` file (the `["CSV"]["dir"]` field). The output folder will contain two files after running PKDuck: `simstring_pkduck.csv` and `auxiliary_pkduck.csv`. `simstring_pkduck.csv` contains all similar pairs of input strings identified by PKDuck. `auxiliary_pkduck.csv` contains all occurrences of all strings appeared in `simstring_pkduck.csv` -- each row contains a string, the name of the table it appears in, the row id, column id and column name.  

### Specify Parameters
The following two parameters need to be passed into the `execute_pkduck` function in `pkduck_api.py`:
  * **Similarity threshold**: a real number between 0 and 1. PKDuck will output all pairs of strings whose similarity is beyond the specified threshold. This threshold should generally be bigger than 0.5 to avoid outputting too many irrelevant string pairs. 
  * **Columns**: a list of columns to run PKDuck on. PKDuck will merge all these columns together and identify similar string pairs among them. This column list should be encoded into a string as follows: the column ids (starting from 0) in the same table are encoded together separated by comma, then the encoded strings of different tables are concatenated together separated by hashtag. For example, to run PKDuck on the 1st and 3rd columns of the 1st table, and the 2nd and 4th columns of the 3rd table, the encoded string is `0,2##1,3`. 

The main function of `pkduck_api.py` shows an example spec based on the example toy input data in the `data` folder. After compiling, directly running `pkduck_api.py` should produce the output files for the toy input without changing `input.json` or `output.json`. 


How to Run PKDuck in the Data Civilizer Studio
-------------
Running PKDuck in the civilizer studio is almost the same as running it independently. First compile the shared library. Then specify input/output folders and parameters in the studio in the same way described above. The only additional work is to specify the input/output config files (the `input.json` and `output.json` files) in the studio. 
