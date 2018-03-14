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

## The Data Civilizer Architecture and User Manual 
Data Civilizer provides several data discovery and cleaning services. The main components of the civilizer is shown in the below figure. Data Civilizer contains three primary layers, namely the CIVILIZER frontend, the CIVILIZER engine, and the CIVILIZER services. The workflow engine allows a user to string together any of the CIVILIZER services in a directed graph to accomplish her data integration goal. Then the engine manages the execution of services in the directed graph. Users interact with the system via the Civilizer Studio.

![The Data Civilizer system](dataCivilizerSystem.jpg) 

### In a nutshell, Data Civilizer has the following main services:

#### A data discovery module, Aurum, to help find datasets of interest. Aurum builds an enterprise knowledge graph (EKG) to connect similar tables together. Aurum allows a user to browse the graph, run similarity searches on tables in the graph, and perform keyword searches. Aurum can also help find all join paths that connect these tables. 

#### An entity resolution module  to form clusters of records thought to represent the same entity. We are currently using DeepER, a deep learning-based entity resolution module. This can be easily replaced with another module which can do the same job.

#### A golden record module. This component has the effect of collapsing clusters representing the same entity into single representative records. Our golden record system excels at cleaning when there are multiple sources of the same information.  

#### Other  cleaning  tools  in  the system  include  an  abbreviation system,  a disguised missing values detection tool (fahes), and ImputeDB for filling in missing values. 