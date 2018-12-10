from flask import Flask, request, jsonify
import json
from os import environ as env
import webbrowser
from decimal import Decimal
import time
import random
import sys
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

    if "operators" in task_request:
        operators = task_request["operators"] if "operators" in task_request else task_request
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
        print(service_id, file=sys.stderr)

    for service_id, service in enumerate(ordered_list):
        # the service expects input from a previous service, link the two services
        # index of input dir is always at index 2 of parameters list
        # index of output dir is always at index 3 of parameters list
        if str(service_id) in service_input_dir:
            service['parameters']['param2'] = service_input_dir[str(service_id)]

        for next_service_id in service['next']: # fill out the input dirs for the next services
            # if no out dir was specified, assume in dir is same as out dir
            if not service['parameters']['param3']: 
                service_input_dir[next_service_id] = service['parameters']['param2']
            else:
                service_input_dir[next_service_id] = service['parameters']['param3']

        #Execute service
        executeOperator(service)

    return jsonify(myresponse0)


@app.route('/rheem/plan_exec_op', methods=['POST'])
def post_ExeOperator():
    time.sleep(random.randint(5, 25))
    return jsonify(myresponse0)
#   return executeOperator(request.json)


def executeOperator(operator):
    class_name = operator["java_class"]
    task_sources = operator["parameters"]["param2"]
    task_destination = operator["parameters"]["param3"]
    input_source, output_destination = get_source_destination_objects(task_sources, task_destination)

    if(class_name == "civilizer.basic.operators.DataDiscovery"):
        print("Data Discovery")
        # open_chrome('http://localhost:3000/')
        return jsonify(myresponse2)

    elif(class_name=="civilizer.basic.operators.DataCleaning-Fahes"):
        print("DataCleaning-Fahes")
        fahes_api.execute_fahes(input_source, output_destination)

#   elif(class_name=="civilizer.basic.operators.DataCleaning-FahesFilter"):
#       print("DataCleaning-FahesFilter")
#       fahes_api.executeService(input_source, output_destination)

    elif(class_name=="civilizer.basic.operators.DataCleaning-FahesApply"):
        print("DataCleaning-FahesApply")
        fahes_api.executeService(input_source, output_destination)

    elif (class_name == "civilizer.basic.operators.DataCleaning-PKDuck"):
        print("DataCleaning-PKDuck")
        columns = operator["parameters"]["param4"]
        tau = operator["parameters"]["param5"]
        pkduck_api.execute_pkduck(input_source, output_destination, columns, Decimal(tau))
        # inputF = "sources.json"
        # outputF = "destination.json"
        # columns = "12#11#8#7#1,2,7#10"
        # pkduck_api.execute_pkduck_file(inputF, outputF, columns, 0.8)

    elif (class_name == "civilizer.basic.operators.DataCleaning-Imputedb"):
        print("DataCleaning-Imputedb")
        tableName = operator["parameters"]["param4"]
        q = operator["parameters"]["param5"]
        r = operator["parameters"]["param6"]
        input_source = {'CSV': {'dir': task_sources, 'table': tableName}}
        # imputedb_api.execute_imputedb(input_source, output_destination, q, r)
        imputedb_api.executeService(input_source, output_destination, q, r)
        # inputF = "sources_im.json"
        # outputF = "destination.json"
        # imputedb_api.execute_imputedb_file(inputF, outputF, 'select Dept_Budget_Code from Sis_department;', 0)

    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER"):
        print("DataCleaning-DeepER")
        table1 = operator["parameters"]["param4"]
        table2 = operator["parameters"]["param5"]
        predictionsFileName = operator["parameters"]["param6"]
        number_of_pairs = operator["parameters"]["param7"]
        deeper_api.execute_deeper(task_sources, table1, table2, number_of_pairs, task_destination, predictionsFileName)

    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER-Train"):
        print("DataCleaning-DeepER-Train")
        dataset_name = operator["parameters"]["param11"]
        params = {
            "dataset_name": dataset_name,
            dataset_name: {
                "dataset_folder_path": operator["parameters"]["param2"],
                "ltable_file_name": operator["parameters"]["param4"],
                "rtable_file_name": operator["parameters"]["param5"],
                "blocking_key": {
                    "l": operator["parameters"]["param7"],
                    "r": operator["parameters"]["param8"]
                },
                "candidates_file": operator["parameters"]["param6"],
                "attribute_list": {
                    "l": operator["parameters"]["param9"].split(","),
                    "r": operator["parameters"]["param10"].split(",")
                }
            },
            "out_file_path": operator["parameters"]["param3"]
        }
        deeper_lite_api.executeServiceTrain(params)

    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER-Predict"):
        print("DataCleaning-DeepER-Predict")
        dataset_name = operator["parameters"]["param11"]
        params = {
            "dataset_name": dataset_name,
            dataset_name: {
                "dataset_folder_path": operator["parameters"]["param2"],
                "ltable_file_name": operator["parameters"]["param4"],
                "rtable_file_name": operator["parameters"]["param5"],
                "blocking_key": {
                    "l": operator["parameters"]["param7"],
                    "r": operator["parameters"]["param8"]
                },
                "candidates_file": operator["parameters"]["param6"],
                "attribute_list": {
                    "l": operator["parameters"]["param9"].split(","),
                    "r": operator["parameters"]["param10"].split(",")
                }
            },
            "out_file_path": operator["parameters"]["param3"]
        }
        deeper_lite_api.executeServicePredict(params)

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
    return jsonify(myresponse0)


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
    port = env.get('PORT')
    if not port: 
        port = "8089"
    app.run(host='0.0.0.0', port=int(port))
