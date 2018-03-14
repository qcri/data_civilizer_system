# data_civilizer_system

Data Civilizer is an end-to-end data preparation system. 

## Demo
A demo could be hosted at URI.

## Docker Installation
You need to install [Docker](https://www.docker.com/community-edition)
and [Docker Compose](https://docs.docker.com/compose/install/)
first, then proceed to the following instructions.

### Development

Get the code

    # clone using https
    git clone --recursive https://github.com/qcri/data_civilizer_system.git
    # or if you prefer ssh
    git clone git@github.com:qcri/data_civilizer_system.git

    cd data_civilizer_system
    
Build and run:

    docker-compose up --build

Now you can visit [http://localhost:5000](http://localhost:5000).

## User Manual 
Data Civilizer provides several data discovery and cleaning services. The main components of the civilizer is shown in the below figure. 

![The Data Civilizer system](dataCivilizerSystem.pdf) 

TODO