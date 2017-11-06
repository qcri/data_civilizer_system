from flask import Flask, request, jsonify
import json
import webbrowser
# from subprocess import Popen, PIPE
from civilizer_services.fahes_service import fahes_api
from civilizer_services.imputedb_service import imputedb_api
from civilizer_services.pkduck_service import pkduck_api
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
    operators = request.json["operators"]
    number = len(operators)
    class_name =operators[number-1]["java_class"]

    if(class_name=="civilizer.basic.operators.DataDiscovery"):
        print("Data Discovery")
        open_chrome('http://localhost:3000/')
    elif(class_name=="civilizer.basic.operators.DataCleaning-Fahes"):
        print("DataCleaning-Fahes")
        inputF = "sources.json"
        outputF = "destination.json"
        fahes_api.execute_fahes(inputF, outputF)
    elif (class_name == "civilizer.basic.operators.DataCleaning-PKDuck"):
        print("DataCleaning-PKDuck")
        inputF = "sources.json"
        outputF = "destination.json"
        columns = "12#11#8#7#1,2,7#10"
        pkduck_api.execute_pkduck(inputF, outputF, columns, 0.8)
    elif (class_name == "civilizer.basic.operators.DataCleaning-Imputedb"):
        print("DataCleaning-Imputedb")
        inputF = "sources_im.json"
        outputF = "destination.json"
        imputedb_api.execute_imputedb(inputF, outputF, 'select Dept_Budget_Code from Sis_department;', 0)
    elif(class_name == "civilizer.basic.operators.EntityConsolidation"):
        print("Data Discovery")
        open_chrome('http://localhost:8888/notebooks/civilizer_gr.ipynb')
    else:
        print("Error")
    return jsonify(operators[number-1])


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
    aurum_api.init()

if __name__ == '__main__':
    # app.run(debug=True)
    # init_modules()
    app.run(host='localhost', port=8089)
