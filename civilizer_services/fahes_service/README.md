## FAHES
#### FAHES which took its name from the Arabic word that means inspector, is a tool for detecting the disguised missing values. By disguised missing values, we refer to the values in a table that replaces the missing values.
#### FAHES is a service integrated with the data civilizer system that can work under the Civilizer Studio or it can work as a stand-alone tool.
#### To compile FAHES, change to directory fahes/ and simply type make from the terminal. 
$ make
#### This will produce a library called "libFahes.so", which can be accessed through the python script "fahes_api.py"

### fahes_api.py requires the following parameters
#### Input: a json file that contains a path to the directory that include the CSV files, if the table name is specified, the tool will work on the specified table. Otherwise, the tool will search for all csv files inside the specified directory.

#### Output: a json file that specifies the place where to put the output table.

The output tables contain meta-data that describes the disguised missing values. Each file contain four columns:
Table Name
Column Name 
The value that detected as disguised missing value
The frequency (the number of occurrences of the value in that column)

#### To run FAHES as a stand-alone tool, change to the fahes_service folder and run the python script "fahes_api.py". Tou will need to make the following modifications
Uncomment the line that contains "tool_loc = "./fahes/""
Comment the line that include "tool_loc = "../civilizer_services/fahes_service/fahes/""
In "sources.json", modify the "dir" to include the path to your data (csv files). If you like to work on specific tables, add the names of the tables in the field "table". Separate the tables names by semicolon (;). When the name of the table is left empty, the tool will work on all the csv files inside the specified directory.

$ cd data_civilizer_system/civilizer_services/fahes_service/fahes/
$ make
$ cd ..
$ python3 fahes_api.py

