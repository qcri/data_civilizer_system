########################################
import os
import json
import re
import csv
import subprocess
########################################


########################################
def execute_deeper():
    """
    This method runs deeper on a dataset.
    """
    params=["/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/Amazon-GoogleProducts",
            "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/Amazon_full.csv",
            "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/GoogleProducts_full.csv",
            "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/predPairs_before.csv",
            "1000000",
            "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/output/predictions_out.csv",
            "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/threshold_06.txt",
            "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/output"
            ]

    tool_path="/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/civilizer_services/deeper_service/DeepER-Lite/"

    # command = [tool_path + "{}/dBoost/dboost/dboost-stdin.py".format(TOOLS_FOLDER), "-F", ",",dataset_path] + dboost_parameters
    # p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    # p.communicate()

    command = [tool_path+"run-2.sh"]
    command.extend(params)
    print(command)
    # p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    p = subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0]
    # p.communicate()
    print(p)





