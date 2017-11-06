## FAHES
#### FAHES which took its name from the Arabic word that means inspector, is a tool for detecting the disguised missing values. By disguised missing values, we refer to the values in a table that replaces the missing values.
#### FAHES is a service integrated with the data civilizer system that can work under the Civilizer Studio or it can work as a stand-alone tool.
#### To compile FAHES, change to directory fahes/ and simply type make from the terminal. 
$ make
#### This will produce a library called "libFahes.so", which can be accessed through the python script "fahes_api.py"

### fahes_api.py requires the follwing parameters
#### Input: a json file that contains a path to the directory that include the CSV files, if the table name is specified, the tool will work on the specified table. Otherwise, the tool will search for all csv files inside the specified directory
