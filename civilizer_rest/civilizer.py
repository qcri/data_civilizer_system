from flask import Flask, request, jsonify
import json
import webbrowser
from decimal import Decimal
# from subprocess import Popen, PIPE
from civilizer_services.fahes_service import fahes_api
from civilizer_services.imputedb_service import imputedb_api
from civilizer_services.pkduck_service import pkduck_api
# from civilizer_services.cleaning_service import cleaning_api
from civilizer_services.deeper_service import deeper_api
# from civilizer_services.aurum_service import aurum_api



app = Flask(__name__)


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


@app.route('/rheem/plan_executions', methods=['POST'])
def post_ExePlan():
    # Posted JSON Plan
    task_request = request.json
    operators = task_request["operators"]
    number = len(operators)
    task_sources = operators[number - 1]["parameters"]["param1"]
    task_destination = operators[number - 1]["parameters"]["param2"]
    input_source, output_destination = get_source_destination_objects(task_sources, task_destination)
    class_name = operators[number-1]["java_class"]


    if(class_name=="civilizer.basic.operators.DataDiscovery"):
        print("Data Discovery")
        open_chrome('http://localhost:3000/')
    elif(class_name=="civilizer.basic.operators.DataCleaning-Fahes"):
        print("DataCleaning-Fahes")
        fahes_api.execute_fahes(input_source, output_destination)
    elif (class_name == "civilizer.basic.operators.DataCleaning-PKDuck"):
        print("DataCleaning-PKDuck")
        columns = operators[number - 1]["parameters"]["param3"]
        tau = operators[number - 1]["parameters"]["param4"]
        pkduck_api.execute_pkduck(input_source, output_destination, columns, Decimal(tau))
        # inputF = "sources.json"
        # outputF = "destination.json"
        # columns = "12#11#8#7#1,2,7#10"
        # pkduck_api.execute_pkduck_file(inputF, outputF, columns, 0.8)
    elif (class_name == "civilizer.basic.operators.DataCleaning-Imputedb"):
        print("DataCleaning-Imputedb")
        tableName = operators[number - 1]["parameters"]["param3"]
        q = operators[number - 1]["parameters"]["param4"]
        r = operators[number - 1]["parameters"]["param5"]
        input_source = {'CSV': {'dir': task_sources, 'table': tableName}}
        imputedb_api.execute_imputedb(input_source, output_destination, q, r)
        # inputF = "sources_im.json"
        # outputF = "destination.json"
        # imputedb_api.execute_imputedb_file(inputF, outputF, 'select Dept_Budget_Code from Sis_department;', 0)
    elif (class_name == "civilizer.basic.operators.DataCleaning-Profiler"):
        print("DataCleaning-Profiler")
        inputF = "sources_p.json"
        outputF = "destination.json"
        # cleaning_api.execute_cleaning(inputF, outputF)
    elif (class_name == "civilizer.basic.operators.EntityMatching-DeepER"):
        print("DataCleaning-DeepER")
        deeper_api.execute_deeper()
    elif(class_name == "civilizer.basic.operators.EntityConsolidation"):
        print("Data Discovery")
        open_chrome('http://localhost:8888/notebooks/civilizer_gr.ipynb')
    else:
        print("Error")
    return jsonify(operators[number-1])


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
    json_file = 'operators.json'
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
    app.run(host='localhost', port=8089)
