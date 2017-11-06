# data_civilizer_system
## Running Data Civilizer
### you need to start the following main servers
### MangoDB which is used by the Studio to store the workflows
#### move to storage/mongos
#### run mongod --dbpath .
### Start the Studio 
#### run gulp start 
#### it is running at port http://localhost:5000/
### Start the Civilizer Rest
#### move to civilizer_rest
#### run python civilizer.py
