from flask import Flask, request, jsonify
import json
from os import environ as env
from decimal import Decimal
from call_goldenrecord import main as M

app = Flask(__name__)

myresponse0 = {'myURI': u''}

@app.route('/rheem/plan_executions', methods=['POST'])
def post_ExePlan():
    # Posted JSON Plan
    task_request = request.json
    operators = task_request["operators"]
    # number = len(operators)
    number = get_activeNode(request) + 1
    task_sources = operators[number - 1]["parameters"]["param2"]
    task_destination = operators[number - 1]["parameters"]["param3"]
    input_source, output_destination = get_source_destination_objects(task_sources, task_destination)
    class_name = operators[number-1]["java_class"]

    if(class_name == "civilizer.basic.operators.EntityConsolidation"):
        print("Entity Consolidation")
        preput = operators[number - 1]["parameters"]["param4"]
        m_file = "matches.csv"
        file = task_sources +  m_file
        with open(task_destination, "w") as my_empty_csv:
            pass  # or write something to it already
        M(task_sources, task_destination, preput)
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
        else:
            break 
    return active_node_index

def get_source_destination_objects(s, d):
    source = {'CSV': {'dir': s, 'table': ''}}
    destination = {'CSV': {'dir': d}}
    return source, destination

if __name__ == '__main__':
    port = env.get('PORT')
    if not port: 
        port = "8889"
    app.run(host='0.0.0.0', port=int(port))
