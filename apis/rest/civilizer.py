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
import subprocess

# from subprocess import Popen, PIPE
# demo-g# from services.fahes_service import fahes_api
# demo-g# from services.imputedb_service import imputedb_api
# demo-g# from services.pkduck_service import pkduck_api
# from services.cleaning_service import cleaning_api
# demo-g# from services.deeper_service import deeper_api
# from services.aurum_service import aurum_api
# demo-g# from services.deeper_lite_service import
# deeper_lite_sim_train_predict as deeper_lite_api

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


def log_text(text):
    log_path = '/app/'
    file_name = log_path + 'log.txt'
    f = open(file_name, "a+")
    f.write(text)
    f.write('\n')
    f.close


def log(text):
    print(text)
    if docker_log is not None:
        print(text, file=docker_log)


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
    re_query = re.compile(
        "^\\s*SELECT\\s*([^;]*\\S)\\s*FROM\\s*([^;]*\\S)\\s*;\\s*$", re.IGNORECASE)
    match = re_query.match(query)
    if not match:
        raise SyntaxError(
            "Unrecognized column name query, '{0}'.".format(query))

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
    if columns[0] == '*':
        columns = headers.copy()
    for i, column in enumerate(columns):
        try:
            columns[i] = str(headers.index(column))
        except ValueError:
            raise NameError(
                "Unrecognized column name '{0}' for table '{1}'.".format(column, table_name))

    return filepath[0], table_name, ",".join(columns)


@app.route('/rheem/rheem_plans', methods=['POST'])
def post_plan():
    # Posted JSON Plan
    return jsonify(request.json)


# Returns adjacent vertices (next services) for a given service node in
# the pipeline
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
            order_tasks(service_id_json_map[
                        next_service_id], next_service_id, ordered_list, visited, service_id_json_map)

    ordered_list.insert(0, service)  # Insert current service into the list


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
    # this array is used  to map service ids
    service_id_json_map = [None] * len(task_request)
    # to the service JSON description

    # first, we perform a toplogical sort of the workflow DAG
    visited = [False] * len(pipeline)
    ordered_list = []

    for service_id, service in enumerate(pipeline):
        print(service_id)
        service_id_json_map[service_id] = service

    for service_id, service in enumerate(pipeline):
        if visited[service_id] == False:
            order_tasks(service, service_id, ordered_list,
                        visited, service_id_json_map)

    # this dict is used to keep track of the input dirs of each service
    service_input_dir = {}
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
            service['parameters'][
                'param2'] = service_input_dir[str(service_id)]

        # fill out the input dirs for the next services
        for next_service_id in service['next']:
            # if no out dir was specified, assume in dir is same as out dir
            if 'param3' in service['parameters']:
                if not service['parameters']['param3']:
                    service_input_dir[next_service_id] = service[
                        'parameters']['param2']
                else:
                    service_input_dir[next_service_id] = service[
                        'parameters']['param3']

        log(str(service_id) + ": " + service['name'])
        log("  param2: " + service['parameters']['param2']
            if 'param2' in service['parameters'] else "Not set")
        log("  param3: " + service['parameters']['param3']
            if 'param3' in service['parameters'] else "Not set")

        for key, value in op_response.items():
            service['parameters'][key] = value

        # Execute service
        op_response = executeOperator(service).json

    return jsonify(op_response)
#   return jsonify(myresponse0)


progress = {}


@app.route('/rheem/plan_exec_op', methods=['POST'])
def post_ExeOperator():
    op_request = request.json

    key = None
    if 'features' in op_request and 'progress' in op_request["features"] and op_request["features"]["progress"]:
        run_id = op_request["run_id"]
        name = op_request["name"]
        key = run_id + "." + name

        # To call the proper progress function for a service, we have to know
        # it's class name.  So, if the operator supports advanced progress
        # reporting, we build a key name from the run_id and operator name and
        # track the operator class globally.  We delete this item from the
        # progress tracking dictionary before exiting so as to not leak memory.
        progress[key] = {
            "class_name": op_request["java_class"],
            "progress": 0
        }

    if 'simulate' in op_request and op_request['simulate']:
        log("simulating op_request:")
        log(json.dumps(op_request, sort_keys=True, indent=4))
        if not key:
            # For services that don't support advanced progress reporting,
            # simply wait 2-10 seconds
            time.sleep(random.randint(2, 10))
        else:
            # For services that support advanced progress reporting, simulate
            # inceasing progress over 20 seconds
            for i in range(1, 10):
                progress[key]["progress"] = i * 10 / 100
                time.sleep(2)
        retval = jsonify(myresponse0)
    else:
        retval = executeOperator(op_request)

    if key:
        progress.pop(key, None)

    return retval


def executeOperator(operator):
    global tmpdir

    class_name = operator["java_class"]
    parameters = operator["parameters"]
    inputs = operator["inputs"]
    #
    # WARNING:
    #
    # The "inputs" property of the operator used to be an array where each
    # index corresponded to a physical input connection point on the workflow
    # chart and the value stored in each array element was the output object
    # from the preceding operator linked to that connection point.
    #
    # This has now been changed such that each element of "inputs" is itself
    # an array to allow multiple output objects to be queued to each input
    # connection point.  However, thorough consideration has not been made
    # as to how to handle arbitrary object properties generically.
    #
    # The code below collapses multiple input objects in each inputs element
    # into a single object so that subsequent operator processing occurs as
    # before.  Primarily, this checks for and concatenates the contents of the
    # "civilizer.dataCollection.filelist" property from each input object into
    # the first.  Any other properties from input objects beyond the first are
    # ignored.  Other properties on the first input object are left unchanged
    # and passed to the operator for processing.
    #
    for index, inputq in enumerate(inputs):
        if len(inputq) > 1:
            if "civilizer.dataCollection.filelist" in inputq[0]:
                for input in inputq[1:]:
                    if "civilizer.dataCollection.filelist" in input:
                        inputq[0]['civilizer.dataCollection.filelist'].extend(
                            input['civilizer.dataCollection.filelist'])
        inputs[index] = inputq[0]

    task_sources = parameters["param2"] if 'param2' in parameters else ""
    task_destination = parameters["param3"] if 'param3' in parameters else ""
    input_source, output_destination = get_source_destination_objects(
        task_sources, task_destination)

    log("input: " + operator['name'])
    log(json.dumps(operator, sort_keys=True, indent=4))

    parameters['civilizer.dataCollection.tmpdir'] = tmpdir + \
        operator['run_id'] + "/"

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
        out_name = parameters["param2"] or "civilizer.dataCollection.filelist"
        output = {
            out_name: list(map(os.path.abspath, files_out))
        }

    elif(class_name == "civilizer.basic.operators.CollectionSink"):
        log("CollectionSink")
        dir_out = parameters["civilizer.collectionSink.location"]
        in_name = parameters["param2"] or "civilizer.dataCollection.filelist"
        try:
            if not os.path.isdir(dir_out):
                os.makedirs(dir_out)
            for file_in in inputs[0][in_name]:
                shutil.copy(file_in, dir_out)
            output = {}
        except OSError as err:
            output = {"error": "OSError: {0}".format(err)}

    elif(class_name == "civilizer.basic.operators.Gather"):
        log("Gather")
        filelist = []
        for input in inputs:
            if 'civilizer.dataCollection.filelist' in input:
                filelist.extend(input['civilizer.dataCollection.filelist'])
        output = {
            'civilizer.dataCollection.filelist': filelist
        }

    elif(class_name == "civilizer.basic.operators.BlackBox2"):
        from shutil import copyfile
        log_text('BlackBox2')
        log_text(parameters["param4"])

        command = parameters["param5"]
        if command:
            subprocess.run([command])

        filelist = []

        try:
            out_dir_path = getOutputDirectory(parameters)
        except OSError as err:
            return {"error": "OSError: {0}".format(err)}

        if not os.path.exists(out_dir_path):
            os.makedirs(out_dir_path)

        for input in inputs:
            if 'civilizer.dataCollection.filelist' in input:
                filelist.extend(input['civilizer.dataCollection.filelist'])
                for file in input['civilizer.dataCollection.filelist']:
                    file_tmp = out_dir_path + file.split("/")[-1]
                    copyfile(file, file_tmp)
                    filelist.append(file_tmp)

        output = {
            'civilizer.dataCollection.filelist': filelist
        }


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


@app.route('/rheem/progress', methods=['GET'])
def get_Progress():
    run_id = request.args.get('run_id')
    names = request.args.getlist('name')
    retval = {}
    for name in names:
        key = run_id + "." + name
        if key in progress:
            # Branch out to service specific progress functions here.
            #
            # Gather only reports as supporting advanced progress when
            # testing...
            if progress[key]["class_name"] == "civilizer.basic.operators.Gather":
                retval[name] = progress[key]
    return jsonify(retval)


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
