from projects import *

##########################
# Service Data Discovery #
##########################

class DataDiscovery(object): #superclass, inherits from default object
    def getName(self):
        raise NotImplementedError
    
class Aurum(DataDiscovery): #subclass, inherits from SuperHero
    def getName(self):
        return "Aurum for Data Discovery" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables
    

##########################
# Service Join Discovery #
##########################

class JoinDiscovery(object): #superclass, inherits from default object
    def getName(self):
        raise NotImplementedError
    
class DongJoin(JoinDiscovery): #subclass, inherits from SuperHero
    def getName(self):
        return "DongJoin for Join Discovery" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables
    

###############################
# Service Data Transformation #
###############################

class DataTransformation(object): #superclass, inherits from default object
    def getName(self):
        raise NotImplementedError
    
class DBXFormer(DataTransformation): #subclass, inherits from SuperHero
    def getName(self):
        return "DBXFormer for Data Transformation" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables

class WenboMethod(DataTransformation): #subclass, inherits from SuperHero
    def getName(self):
        return "Wenbo's method for Data Transformation" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables
    
    
###########################
# Service Entity Matching #
###########################

class EntityMatching(object): #superclass, inherits from default object
    def getName(self):
        raise NotImplementedError
    
class Tamr(EntityMatching): #subclass, inherits from SuperHero
    def getName(self):
        return "Tamr for Entity Matching" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables
    
    
################################
# Service Entity Consolidation #
################################

class EntityConsolidation(object): #superclass, inherits from default object
    def getName(self):
        raise NotImplementedError
    
class GoldenRecord(EntityConsolidation): #subclass, inherits from SuperHero
    def getName(self):
        return "Golden Record for Entity Consolidation" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables
    
    
###########################
# Service Error Detection #
###########################

class ErrorDetection(object): #superclass, inherits from default object
    def getName(self):
        raise NotImplementedError
    
class DetectDisguisedMissingValues(ErrorDetection): #subclass, inherits from SuperHero
    def getName(self):
        return "Detect Disguised Missing Values for Error Detection" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables    
    
class DBoost(ErrorDetection): #subclass, inherits from SuperHero
    def getName(self):
        return "DBoost for Error Detection" 
    def execute(self, inputTables):
        # TODO: Please add the function here to compute and write outputTables
        outputTables = inputTables
        print("Running", self.getName())
        return outputTables        


        