from flask import Flask, request, jsonify
import json
from workflow.civilizer_services.fahes_service import fahes_api



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
    inputF = "sources.json"
    outputF = "destination.json"
    fahes_api.execute_fahes(inputF, outputF)
    return jsonify(request.json)


@app.route('/rheem/rheem_operators', methods=['GET'])
def get_operators():
    # Read the civilizer services
    json_file = 'operators.json'
    json_data = open(json_file)
    operators = json.load(json_data)
    json_data.close()
    return jsonify(operators)


if __name__ == '__main__':
    # app.run(debug=True)
    app.run(host='localhost', port=8089)
