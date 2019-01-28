from flask import Flask, request, jsonify
import json
import time
import random
import os

app = Flask(__name__)

myresponse0 = {'myURI': u''}

tmpdir = "/app/storage/tmp/"

progress = {}

try:
    docker_log = open("/proc/1/fd/1", "w")
except:
    docker_log = None

def log(text):
    print(text)
    if docker_log is not None:
        print(text, file = docker_log)


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
                        inputq[0]['civilizer.dataCollection.filelist'].extend(input['civilizer.dataCollection.filelist'])
        inputs[index] = inputq[0]

    log("input: " + operator['name'])
    log(json.dumps(operator, sort_keys=True, indent=4))

    parameters['civilizer.dataCollection.tmpdir'] = tmpdir + operator['run_id'] + "/"

    output = None

    if(class_name == "civilizer.basic.operators.noop"):
        log("No op")
        return jsonify(myresponse0)

    elif(class_name == "civilizer.basic.operators.Gather2"):
        log("Gather2")
        filelist = []
        for input in inputs:
            if 'civilizer.dataCollection.filelist' in input:
                filelist.extend(input['civilizer.dataCollection.filelist'])
        output = {
            'civilizer.dataCollection.filelist': filelist
        }

        # Sleep for no purpose other than to demonstrate advanced progress reporting
        run_id = operator["run_id"]
        name = operator["name"]
        key = run_id + "." + name
        for i in range(1, 10):
            progress[key]["progress"] = i * 10 / 100
            time.sleep(2)

    else:
        print("Error")

    if output is not None:
        log("output: " + operator['name'])
        log(json.dumps(output, sort_keys=True, indent=4))
    return jsonify(myresponse0 if not output else output)


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
            # Gather only reports as supporting advanced progress when testing...
            if progress[key]["class_name"] == "civilizer.basic.operators.Gather2":
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


if __name__ == '__main__':
    port = os.environ.get('PORT')
    if not port: 
        port = "8090"
    app.run(host='0.0.0.0', port=int(port))
