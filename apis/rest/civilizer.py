from flask import Flask, request, jsonify
import json
import webbrowser
from decimal import Decimal
import time
import random
import os
import glob
import re
import shutil
import tempfile
import uuid

# from subprocess import Popen, PIPE
from services.fahes_service import fahes_api
from services.imputedb_service import imputedb_api
from services.pkduck_service import pkduck_api
# from services.cleaning_service import cleaning_api
from services.deeper_service import deeper_api
# from services.aurum_service import aurum_api
from services.deeper_lite_service import deeper_lite_sim_train_predict as deeper_lite_api

app = Flask(__name__)

myresponse1 = {'myURI': u'http://0.0.0.0:8888/notebooks/civilizer_gr.ipynb'}
myresponse2 = {'myURI': u'http://localhost:3000/'}
myresponse0 = {'myURI': u''}
# operators = [
#     {
#         'class': u'org.qcri.civilizer.basic.operators.DataDiscovery',
#         'parameters': {'0': {'0': {'name': u'inputTypeClass', 'type': u'type', 'inputType': u'String'}}},
#         'nb_inputs': 2,
#         'nb_outputs': 1
#     },
#     {
#         'class': u'org.qcri.civilizer.basic.operators.JoinDiscovery',
#         'parameters': {'0': {'0': {'name': u'inputTypeClass', 'type': u'type', 'inputType': u'String'}}},
#         'nb_inputs': 2,
#         'nb_outputs': 1
#     },
# ]

tmpdir = "/app/storage/tmp/"

try:
    docker_log = open("/proc/1/fd/1", "w")
except:
    docker_log = None

def log(text):
    print(text)
    if docker_log is not None:
        print(text, file = docker_log)

def getOutputDirectory(parameters):
    tmpdir = ""
    if 'civilizer.dataCollection.tmpdir' in parameters:
        tmpdir = parameters['civilizer.dataCollection.tmpdir']
    if not tmpdir:
        tmpdir = tempfile.gettempdir()
    output_dir = os.path.abspath(tmpdir + "/" + str(uuid.uuid4()))

    # Will raise an exception if output_dir already exists or cannot be created
    os.makedirs(output_dir)

    return output_dir + "/"

def parseQuery(filelist, query, returnColumns=False):
    # Use regex to perform simplistic query parsing
    re_query = re.compile("^\\s*SELECT\\s*([^;]*\\S)\\s*FROM\\s*([^;]*\\S)\\s*;\\s*$", re.IGNORECASE)
    match = re_query.match(query)
    if not match:
        raise SyntaxError("Unrecognized column name query, '{0}'.".format(query))

    # Identify the CSV from filelist associated with the table name from query
    table_name = match.group(2)
    re_table = re.compile("/" + re.escape(table_name) + "\\.[Cc][Ss][Vv]$")
    filepath = [x for x in filelist if re_table.search(x)]
    if len(filepath) == 0:
        raise NameError("Unrecognized table name, '{0}'.".format(table_name))
    if len(filepath) > 1:
        raise NameError("Ambiguous table name, '{0}'.".format(table_name))

    if not returnColumns:
        return filepath[0], table_name

    # Read the header line of the CSV file
    with open(filepath[0]) as f:
        headers = [x.strip() for x in f.readline().split(",")]

    # Convert the column names from query to index numbers from file headers
    columns = [x.strip() for x in match.group(1).split(",")]
    for i, column in enumerate(columns):
        try:
            columns[i] = str(headers.index(column))
        except ValueError:
            raise NameError("Unrecognized column name '{0}' for table '{1}'.".format(column, table_name))

    return filepath[0], table_name, ",".join(columns)

@app.route('/rheem/rheem_plans', methods=['POST'])
def post_plan():
    # Posted JSON Plan
    return jsonify(request.json)


# Returns adjacent vertices (next services) for a given service node in the pipeline
def get_next_services(service):
    return service['next']


# Topological ordering of the workflow graph
# JSON of ordered services is inserted into ordered_list

def order_tasks(service, service_id, ordered_list, visited, service_id_json_map):
    print(service_id)
    visited[service_id] = True

    for next_service in get_next_services(service):
        next_service_id = int(next_service)
        if (visited[next_service_id] == False):
            order_tasks(service_id_json_map[next_service_id], next_service_id, ordered_list, visited, service_id_json_map)

    ordered_list.insert(0, service) # Insert current service into the list


@app.route('/rheem/plan_executions', methods=['POST'])
def post_ExePlan():
    # Posted JSON Plan
    task_request = request.json

    log("task_request:")
    log(json.dumps(task_request, sort_keys=True, indent=4))

    # To support legacy plan execution, where an entire rheem plan is passed
    # and the "active" operator is indicated with param1 == 'y'
    if "operators" in task_request:
        operators = task_request["operators"]
        # number = len(operators)
        number = get_activeNode(request)
        return executeOperator(operators[number])

    pipeline = task_request
    service_id_json_map = [None]*len(task_request) # this array is used  to map service ids
                                               # to the service JSON description

    # first, we perform a toplogical sort of the workflow DAG
    visited = [False]*len(pipeline)
    ordered_list = []

    for service_id, service in enumerate(pipeline):
        print(service_id)
        service_id_json_map[service_id] = service

    for service_id, service in enumerate(pipeline):
        if visited[service_id] == False:
            order_tasks(service, service_id, ordered_list, visited, service_id_json_map)

    service_input_dir = {} # this dict is used to keep track of the input dirs of each service
                           # service_input_dir[s] returns the input dir (if any) for service s 

    for service_id, service in enumerate(ordered_list):
        print(service_id)

    op_response = {}
    for service_id, service in enumerate(ordered_list):
        # the service expects input from a previous service, link the two services
        # index of input dir is always at index 2 of parameters list
        # index of output dir is always at index 3 of parameters list
        if str(service_id) in service_input_dir:
            log(str(service_id) + " in service_input_dir")
            service['parameters']['param2'] = service_input_dir[str(service_id)]

        for next_service_id in service['next']: # fill out the input dirs for the next services
            # if no out dir was specified, assume in dir is same as out dir
            if 'param3' in service['parameters']:
                if not service['parameters']['param3']: 
                    service_input_dir[next_service_id] = service['parameters']['param2']
                else:
                    service_input_dir[next_service_id] = service['parameters']['param3']

        log(str(service_id) + ": " + service['name'])
        log("  param2: " + service['parameters']['param2'] if 'param2' in service['parameters'] else "Not set")
        log("  param3: " + service['parameters']['param3'] if 'param3' in service['parameters'] else "Not set")

        for key, value in op_response.items():
            service['parameters'][key] = value

        #Execute service
        op_response = executeOperator(service).json

    return jsonify(op_response)
#   return jsonify(myresponse0)


@app.route('/rheem/plan_exec_op', methods=['POST'])
def post_ExeOperator():
    op_request = request.json
    if 'simulate' in op_request and op_request['simulate']:
        log("simulating op_request:")
        log(json.dumps(op_request, sort_keys=True, indent=4))
        time.sleep(random.randint(5, 25))
        return jsonify(myresponse0)
#   log("op_request:")
#   log(json.dumps(op_request, sort_keys=True, indent=4))
    return executeOperator(op_request)


def executeOperator(operator):
    global tmpdir

    class_name = operator["java_class"]
    parameters = operator["parameters"]
    inputs = operator["inputs"]
    task_sources = parameters["param2"] if 'param2' in parameters else ""
    task_destination = parameters["param3"] if 'param3' in parameters else ""
    input_source, output_destination = get_source_destination_objects(task_sources, task_destination)

    log("input: " + operator['name'])
    log(json.dumps(operator, sort_keys=True, indent=4))

    parameters['civilizer.dataCollection.tmpdir'] = tmpdir + operator['run_id'] + "/"

    output = None

    if(class_name == "civilizer.basic.operators.noop"):
        log("No op")
        return jsonify(myresponse0)

    elif(class_name == "civilizer.basic.operators.CollectionSource"):
        log("CollectionSource")
        files_in = parameters["param0"].splitlines()
        files_out = list()
        for files in list(map(glob.glob, files_in)):
            files_out.extend(files)
        output = {
            "civilizer.dataCollection.filelist": list(map(os.path.abspath, files_out))
        }

    elif(class_name == "civilizer.basic.operators.CollectionSink"):
        log("CollectionSink")
        dir_out = parameters["civilizer.collectionSink.location"]
        try:
            if not os.path.isdir(dir_out):
                os.makedirs(dir_out)
            for file_in in inputs[0]['civilizer.dataCollection.filelist']:
                shutil.copy(file_in, dir_out)
            output = {}
        except OSError as err:
            output = { "error": "OSError: {0}".format(err) }

    elif(class_name == "civilizer.basic.operators.Gather"):
        log("Gather")
        filelist = []
        for input in inputs:
            if 'civilizer.dataCollection.filelist' in input:
                filelist.extend(input['civilizer.dataCollection.filelist'])
        output = {
            'civilizer.dataCollection.filelist': filelist
        }

    elif(class_name == "civilizer.basic.operators.DataDiscovery"):
        print("Data Discovery")
        # open_chrome('http://localhost:3000/')
        return jsonify(myresponse2)

    elif(class_name=="civilizer.basic.operators.DataCleaning-Fahes"):
        print("DataCleaning-Fahes")
        if inputs:
            output = fahes_api.execute_fahes_params(parameters, inputs)
        else:
            fahes_api.execute_fahes(input_source, output_destination)

#   elif(class_name=="civilizer.basic.operators.DataCleaning-FahesFilter"):
#       print("DataCleaning-FahesFilter")
#       fahes_api.executeService(input_source, output_destination)

    elif(class_name=="civilizer.basic.operators.DataCleaning-FahesApply"):
        print("DataCleaning-FahesApply")
        if inputs:
            output = fahes_api.executeService_params(parameters, inputs)
        else:
            fahes_api.executeService(input_source, output_destination)

    elif (class_name == "civilizer.basic.operators.DataCleaning-PKDuck"):
        print("DataCleaning-PKDuck")
        if inputs:
            parameters['civilizer.PKDuck.columnSelect'] = parameters['param4'].splitlines()
            parameters['civilizer.PKDuck.tau'] = Decimal(parameters['param5'])
            output = pkduck_api.executeService_params(parameters, inputs)
        else:
            columns = parameters["param4"]
            tau = parameters["param5"]
            pkduck_api.execute_pkduck(input_source, output_destination, columns, Decimal(tau))
        # inputF = "sources.json"
        # outputF = "destination.json"
        # columns = "12#11#8#7#1,2,7#10"
        # pkduck_api.execute_pkduck_file(inputF, outputF, columns, 0.8)

    elif (class_name == "civilizer.basic.operators.DataCleaning-Imputedb"):
        print("DataCleaning-Imputedb")
        if inputs:
            parameters['civilizer.DataCleaning.Imputedb.Query'] = parameters['param5'].splitlines()
            parameters['civilizer.DataCleaning.Imputedb.Ratio'] = parameters['param6']
            output = imputedb_api.executeService_params(parameters, inputs)
        else:
            tableName = parameters["param4"]
            q = parameters["param5"]
            r = parameters["param6"]
            input_source = {'CSV': {'dir': task_sources, 'table': tableName}}
            # imputedb_api.execute_imputedb(input_source, output_destination, q, r)
            imputedb_api.executeService(input_source, output_destination, q, r)
            # inputF = "sources_im.json"
            # outputF = "destination.json"
            # imputedb_api.execute_imputedb_file(inputF, outputF, 'select Dept_Budget_Code from Sis_department;', 0)

    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER"):
        print("DataCleaning-DeepER")
        table1 = parameters["param4"]
        table2 = parameters["param5"]
        predictionsFileName = parameters["param6"]
        number_of_pairs = parameters["param7"]
        deeper_api.execute_deeper(task_sources, table1, table2, number_of_pairs, task_destination, predictionsFileName)

    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER-Train"):
        print("DataCleaning-DeepER-Train")
        if inputs:
            ltable_file_path = inputs[0]['civilizer.dataCollection.filelist'][0]
            rtable_file_path = inputs[0]['civilizer.dataCollection.filelist'][1]
            if not rtable_file_path:
                rtable_file_path = ltable_file_path
        else:
            parameters["metadata_path"] = parameters["param2"]
            ltable_file_path = parameters["param4"]
            rtable_file_path = parameters["param5"]
        parameters["ltable_file_path"] = ltable_file_path
        parameters["rtable_file_path"] = rtable_file_path
        parameters["labeled_file_path"] = parameters["param6"]
#       params = {
#           "metadata_path":parameters["param2"],
#           "ltable_file_path":parameters["param4"],
#           "rtable_file_path":parameters["param5"],
#           "labeled_file_path":parameters["param6"],
#       }
        output = deeper_lite_api.executeServiceTrain(parameters, inputs)

    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER-Predict"):
        print("DataCleaning-DeepER-Predict")
        if inputs:
            parameters["metadata_path"] = inputs[0]["civilizer.DeepER.metadataPath"]
            ltable_file_path = inputs[0]['civilizer.dataCollection.filelist'][0]
            rtable_file_path = inputs[0]['civilizer.dataCollection.filelist'][1]
            if not rtable_file_path:
                rtable_file_path = ltable_file_path
        else:
            parameters["metadata_path"] = parameters["param2"]
            ltable_file_path = parameters["param4"]
            rtable_file_path = parameters["param5"]
        parameters["ltable_file_path"] = ltable_file_path
        parameters["rtable_file_path"] = rtable_file_path
        parameters["candidates_file_path"] = parameters["param6"]
        parameters["lblocking_key"] = parameters["param7"]
        parameters["rblocking_key"] = parameters["param7"]
#       params = {
#               "metadata_path":parameters["param2"],
#               "out_file_path":parameters["param3"],
#               "ltable_file_path":parameters["param4"],
#               "rtable_file_path":parameters["param5"],
#               "candidates_file_path":parameters["param6"],
#               "lblocking_key":parameters["param7"],
#               "rblocking_key":parameters["param7"],
#       }
        output = deeper_lite_api.executeServicePredict(parameters, inputs)

    elif(class_name == "civilizer.basic.operators.EntityConsolidation"):
        print("Entity Consolidation")
        # gr_source_file = "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/civilizer_services/grecord_service/source.txt"
        # gr_destination_file = "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/civilizer_services/grecord_service/destination.txt"

        # gr_source_file = "/app/rest/services/grecord_service/source.txt"
        # gr_destination_file = "/app/rest/services/grecord_service/destination.txt"

        gr_source_file = "/app/storage/data_sets/gr/source.txt"
        gr_destination_file = "/app/storage/data_sets/gr/destination.txt"

        sfile = open(gr_source_file, 'w')
        sfile.write(task_sources+" ")
        sfile.close()

        dfile = open(gr_destination_file, 'w')
        dfile.write(task_destination+" ")
        dfile.close()

        # open_chrome('http://0.0.0.0:8888/notebooks/civilizer_gr.ipynb')
        return jsonify(myresponse1)
    
    elif (class_name == "civilizer.basic.operators.DataCleaning-Profiler"):
        print("DataCleaning-Profiler")
        inputF = "sources_p.json"
        outputF = "destination.json"
        # cleaning_api.execute_cleaning(inputF, outputF)

    else:
        print("Error")

    # return jsonify(operators[number-1])
    if output is not None:
        log("output: " + operator['name'])
        log(json.dumps(output, sort_keys=True, indent=4))
    return jsonify(myresponse0 if not output else output)


def get_activeNode(request):
    
    active_node_index = 0
    task_request = request.json
    operators = task_request["operators"]
    number = len(operators)
    # x ranges from 0 to number-1
    for x in range(number):    
        isActive = operators[x]["parameters"]["param1"]
        if isActive == 'y':
            active_node_index = x
#       else:
            break 

    return active_node_index


def get_source_destination_objects(s, d):
    source = {'CSV': {'dir': s, 'table': ''}}
    destination = {'CSV': {'dir': d}}
    return source, destination


def open_chrome(url):
    # url = 'http://localhost:3000/'
    # MacOS
    chrome_path = 'open -a /Applications/Google\ Chrome.app %s'
    print("open chrome")
    webbrowser.get(chrome_path).open(url)



# def __placeholder():
#     # FIXME: placeholder -- Aurum invocations
#     keyword = "<keyword from params>"
#     str_tables = run_query1_keyword(keyword)
#     schema_name= "<schema_name from params>"
#     str_tables = run_query2_schema(schema_name)
#     json_obj = dict()
#     json_obj["CSV"] = dict()
#     json_obj["CSV"]["dir"] = "<dir for tables, fix from config>"
#     json_obj["CSV"]["tables"] = str_tables
#     return jsonify(json_obj)


@app.route('/rheem/rheem_operators', methods=['GET'])
def get_operators():
    # Read the civilizer services
    json_file = '/app/rest/operators.json'
    json_data = open(json_file)
    operators = json.load(json_data)
    json_data.close()
    return jsonify(operators)

def init_modules():
    """
    Before the Flask app starts, we call init() function of each module.
    This allows the modules to initialize themselves before the app is
    up for the user.
    """
    # aurum_api.init()

if __name__ == '__main__':
    # app.run(debug=True)
    # init_modules()
    port = os.environ.get('PORT')
    if not port: 
        port = "8089"
    app.run(host='0.0.0.0', port=int(port))
