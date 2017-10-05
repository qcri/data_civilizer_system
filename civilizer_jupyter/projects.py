import os
import shutil, sys
from graphviz import *
from IPython.display import Image, display
import pydot
from services import *
from data_reader import *
##########################
# Functions for projects #
##########################
WorkingDir = 'CivilizerWorkspace/'

class node:
    """Class to store information about individual components in the project"""
    global WorkingDir
    node_functionality = ""
    input_sources = ""
    input_file = ""
    output_file = ""
    tracking_file = "" 
    
    def createNode(self, node_name, node_type, Project):
        self.node_functionality = node_type
        inputF = WorkingDir + Project + '_' + node_name + '.input'
        self.input_file = os.path.abspath(inputF) 
        outputF = WorkingDir + Project + '_' + node_name + '.output'
        self.output_file = os.path.abspath(outputF)
        tracker = WorkingDir + Project + '_' + node_name + '.tracker'
        self.tracking_file = os.path.abspath(tracker)
    
    def printNode(self):
        print("Node class:", self.node_functionality)
        print("input file:", self.input_file)
        print("output file:", self.output_file)
        print("tracking file:", self.tracking_file)
        print("input sources:", self.input_sources)
        print("==================================")

class Project:
    """Class to store project information """
    Project_Name = ""
    wg = None
    nodes_details = dict()
    global WorkingDir
    """ Printing the nodes and eges as string """
    def printProject(self):
        if self.wg:
            wg_str = self.wg.to_string()
            print(wg_str)
        else:
            print("Error: either the project is empty or it has been closed ...")

    def newProject(self, P_Name):
        Project_Directory = WorkingDir + P_Name
        self.Project_Name = P_Name
        if not os.path.exists(WorkingDir):
            os.makedirs(WorkingDir)
        if not os.path.exists(Project_Directory):
            os.makedirs(Project_Directory)
            print ("New project created successfully\n")
        if self.nodes_details:
            for n in self.wg.get_nodes():
                if self.nodes_details.get(n.get_name()):
                    del self.nodes_details[n.get_name()]
        self.wg = pydot.Dot(type = 'digraph')
        
    def loadProject(self, P_Name):
        if self.Project_Name:
            self.saveProject(1);
            self.closeProject(1)
        input_file = WorkingDir + P_Name +'/' + P_Name + '.dot'
        print ("load an existing project from: " + input_file + "\n")
        oldwg = pydot.graph_from_dot_file(input_file)[0]
        self.newProject(P_Name)
        self.wg = oldwg
        if not self.wg:
            print("This is an empty project .. the system will create a new project .. ")
            return
        for n in self.wg.get_nodes():
            newN = node()
            functionality = n.get_label().replace("\"", "")
            newN.createNode(n.get_name(), functionality, self.Project_Name)
            self.nodes_details[n.get_name()] = newN
        for e in self.wg.get_edges():
            nd = self.nodes_details.get(e.get_destination())
            ns = self.nodes_details.get(e.get_source())
            if not (ns.output_file in nd.input_sources):
                nd.input_sources = nd.input_sources + ns.output_file + ";"
                self.nodes_details[e.get_destination()] = nd
                
    def saveProject(self, flag = 0):
        if self.wg:
            output_file = WorkingDir + self.Project_Name +'/' + self.Project_Name + '.dot'
            if not flag: 
                print ("save the current project to: " + output_file + "\n")
            self.wg.write(output_file)

        
    def closeProject(self, flag = 0):
        self.saveProject(1)
        self.Project_Name = ""
        for n in self.wg.get_nodes():
            if self.nodes_details.get(n.get_name()):
                del self.nodes_details[n.get_name()]
            self.wg.del_node(n)
        for e in self.wg.get_edges():
            self.wg.del_edge(e.get_source(), e.get_destination())
        self.wg = None
        if not flag:
            print ("close the current project")

    def deleteProject(self, Proj_Name=None):
        if Proj_Name:
            Project_Directory = WorkingDir + Proj_Name
            if os.path.exists(Project_Directory):
                shutil.rmtree(Project_Directory)
                self.Project_Name = ""
        elif self.Project_Name:
            Project_Directory = WorkingDir + self.Project_Name
            Proj_Name = self.Project_Name
            if os.path.exists(Project_Directory):
                shutil.rmtree(Project_Directory)
                self.Project_Name = ""
                for n in self.wg.get_nodes():
                    if self.nodes_details.get(n.get_name()):
                        del self.nodes_details[n.get_name()]
                    self.wg.del_node(n)
                for e in self.wg.get_edges():
                    self.wg.del_edge(e.get_source(), e.get_destination())
                self.wg = None               
                
                print ("Project ( ", Proj_Name, ") has been deleted successfully .. ")
        else:
            print("Project is not available or it has been closed .. ")

    """
        Adding a new node requires creating an object of the class 'node' to hold the 
        information about the node. This object is added to the nodes_details list
        and a node is added to the working graph
    """
    def addNode(self, node_name, functionality):
        if not self.wg:
            print("Error: Node cannot be added .. project is not available or it has been closed .. ")
            return
        Nodes = self.wg.get_nodes()
        for n in Nodes:
            if n.get_name() == node_name:
                print("Error: node (", node_name, ") was added before .. Please consider using different node name")
                return
        newN = node()
        newN.createNode(node_name, functionality, self.Project_Name)
        new_node = pydot.Node(node_name, label=functionality)
        self.wg.add_node(new_node)
        self.nodes_details[node_name] = newN

    """
        Deleting a given node requires deleting its information in nodes_details, 
        the edges from and to that node and finally removing the node from the graph
    """
    def deleteNode(self, node_name):
        if not self.wg:
            print("Error: Node cannot be added .. project is not available or it has been closed .. ")
            return
        Nodes = self.wg.get_nodes()
        reqNode = None
        for n in Nodes:
            if n.get_name() == node_name:
                reqNode = n
                break
        if not reqNode:
            print("Error: attempting to delete non-existing node .. ")
            return 
        Edges = self.wg.get_edges()
        for e in Edges:
            if ((e.get_source() == node_name) or (e.get_destination() == node_name)):
                self.wg.del_edge(e.get_source(), e.get_destination())
                # return
        self.wg.del_node(reqNode)
        if self.nodes_details.get(node_name):
            del self.nodes_details[node_name]
        print("Node (", node_name, ") was deleted successfully .. ")

    """Remove an edge from the graph"""
    def deleteEdge(self, from_node, to_node):
        if not self.wg:
            print("Error: attempting to delete non-existing edge .. ")
            return
        Edges = self.wg.get_edges()
        for e in Edges:
            if ((e.get_source() == from_node) and (e.get_destination() == to_node)):
                file_to_remove = self.nodes_details[from_node].output_file
                self.nodes_details[to_node].input_sources.replace(file_to_remove, "")
                self.wg.del_edge(from_node, to_node)
                return
        print("Error: attempting to delete non-existing edge .. ")

    """Check if the edge already exist or not """
    def edge_exists(self, from_node, to_node):
        Edges = self.wg.get_edges()
        for e in Edges:
            if ((e.get_source() == from_node) and (e.get_destination() == to_node)):
                print("Error: Edge already exists .. ")
                return 1
        return 0

    def addEdge(self, from_node, to_node):
        edge_nodes = []
        """Check if the working graph has been created or not """
        if not self.wg:
            print("Error: Edge cannot be added .. project is not available or it has been closed .. ")
            return

        if (self.edge_exists(from_node, to_node)): 
            return 
        Nodes = self.wg.get_nodes()
        """ Search for 'from_node' int the graph """
        node_exist = 0
        for e in Nodes:
            if e.get_name() == from_node:
                node_exist = 1
                edge_nodes.append(e)
                break;
        if not node_exist: 
            print("Error: node (", from_node, ") doesn\'t exist")
            return 
        """ Search for 'to_node' int the graph """
        node_exist = 0
        for e in Nodes:
            if e.get_name() == to_node:
                node_exist = 1
                edge_nodes.append(e)
                break;
        if not node_exist: 
            print("The node (", to_node, ") doesn\'t exist")
            return 
        self.wg.add_edge(pydot.Edge(edge_nodes[0], edge_nodes[1]))

        # print ("Modifying the list of sources")
        nd = self.nodes_details.get(to_node)
        ns = self.nodes_details.get(from_node)
        if not (ns.output_file in nd.input_sources):
            nd.input_sources = nd.input_sources + ns.output_file + ";"
            self.nodes_details[to_node] = nd
            

    # def createEdge(from_node, to_node):
    def addNodeInput(self, node_name, newInputFile):
        if not self.wg:
            print("Error (Empty Project): project is not available or it has been closed .. ")
            return
        inputFileAdd = os.path.abspath(newInputFile)
        nd = self.nodes_details.get(node_name)
        if nd:
            nd.input_sources = nd.input_sources + inputFileAdd + ";"
    
    def read_tables(self, tables_list):
        tNames = []
        dFrames = []
        if tNames:
            for i in range(len(tNames)):
                tNames.remove(tNames[0])
        if dFrames:
            for i in range(len(dFrames)):
                dFrames.remove(dFrames[0])
        if not tables_list:
            print("Empty list of table")
            return None, None
        for T in tables_list:
            if T.lower() == 'csv':
                if not (tables_list[T]['table']):
                    Ts, DFs = read_csv_directory(tables_list[T]['dir'])
                    if Ts:
                        for i in range(len(Ts)):
                            tNames.append(Ts[i])
                            dFrames.append(DFs[i])
                else:
                    if not tables_list[T]['dir'].endswith('/'):
                        tables_list[T]['dir'] = tables_list[T]['dir'] + '/'
                    tables = tables_list[T]['table'].split(';')
                    for table in tables:
                        tab_name = tables_list[T]['dir'] + table
                        Ti, DFi = read_csv_file(tab_name)
                        tNames.append(Ti)
                        dFrames.append(DFi)
            elif T.lower() == 'postgres':
                try:
                    dbase = tables_list[T]['database']
                    usr = tables_list[T]['user']
                    pw = tables_list[T]['password']
                    hst = tables_list[T]['host']
                    prt = tables_list[T]['port']
                    tbls = tables_list[T]['table']
                except:
                    print("Missing fields .. cannot access the database")
                    continue
                if not (tbls):
                    Ts, DFs = read_DB('postgres', dbase, hst, prt, usr, pw)
                    for i in range(len(Ts)):
                        tNames.append(Ts[i])
                        dFrames.append(DFs[i])
                else:
                    tables = tbls.split(';')
                    for table in tables:
                        Ti, DFi = read_DB_table('postgres', dbase, hst, prt, usr, pw, table)
                        tNames.append(Ti)
                        dFrames.append(DFi)
            else:
                print("Unknown data sources .. ")
        return tNames, dFrames

    def read_input_data(self, node_name):
        if not self.nodes_details:
            print("Nodes list is empty .. ")
            return None
        ns = self.nodes_details[node_name]
        if not ns:
            print("Node does not exist .. ")
            return None
        sources_list = []
        tablesNames = []
        dataFrames = []
        # Clear the lists if they still has items from previous execution
        if sources_list:
            for i in range(len(sources_list)):
                sources_list.remove(sources_list[0])
        if tablesNames:
            for i in range(len(tablesNames)):
                tablesNames.remove(tablesNames[0])
        if dataFrames:
            for i in range(len(dataFrames)):
                dataFrames.remove(dataFrames[0])
        # ==================
        sources = self.nodes_details[node_name].input_sources
        files = sources.split(';')
        for f_name in files:
            if f_name:
                try:
                    with open(f_name) as data_file:
                        try:    
                            data = json.load(data_file)
                            sources_list.append(data)
                        except:
                            print("Cannot read json file .. (", f_name, ")")
                            return None
                except:
                    print("File not found .. (", f_name, ")")
                    continue
        for element in sources_list:
            T_names, DFs = self.read_tables(element)
            for i in range(len(T_names)):
                tablesNames.append(T_names[i])
                dataFrames.append(DFs[i])
        return tablesNames, dataFrames

    def executeNode(self, node_name):
        target_node = None
        n = self.nodes_details[node_name]
        if not n:
            print("Error, incorrect node_id .. ")
            return
        functionality = n.node_functionality
        X = self.get_class_by_func(functionality)
        X.execute(n.input_file)

    def get_class_by_func(self, fn):
        if fn == 'Aurum': 
            X = Aurum(); return X
        elif fn == 'Tamr':
            X = Tamr(); return X
        elif fn == 'DBXFormer':
            X = DBXFormer(); return X
        elif fn == 'GoldenRecord':
            X = GoldenRecord(); return X
        elif fn == 'DongJoin':
            X = DongJoin(); return X
        elif fn == 'WenboMethod':
            X = WenboMethod(); return X
        elif fn == 'DetectDisguisedMissingValues':
            X = DetectDisguisedMissingValues(); return X
        elif fn == 'DBoost':
            X = DBoost(); return X 
        
            
##################################
#    Operating Workflow graph    #
##################################    


    
##########################
#    Helper Functions    #
##########################    
    
def viewPydot(pdot_graph):
    display(Image(pdot_graph.create_png())) 
    
# Node attributes
# 1. Tracker file for each node


# Tracker file
# 1. Input Tables: a string, by default "null"
# 2. Output Tables: a string, by default "null"
# 3. Function
# 4. Other informaiton for tracking 