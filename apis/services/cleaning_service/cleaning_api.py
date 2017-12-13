########################################
# Abstraction Layer
# Milad Abbaszadeh
# milad.abbaszadehjahromi@campus.tu-berlin.de
# October 2017
# Big Data Management Group
# TU Berlin
# All Rights Reserved
########################################


########################################
import os
import json
import re
import csv
import subprocess
########################################


########################################
DATASETS_FOLDER = "datasets"
TOOLS_FOLDER = "tools"
########################################


########################################
# def install_tools():
#     """
#     This method installs and configures the data cleaning tools.
#     """
#     for tool in os.listdir(TOOLS_FOLDER):
#         if tool == "NADEEF":
#             p = subprocess.Popen(["ant", "all"], cwd="{}/NADEEF".format(TOOLS_FOLDER), stdin=subprocess.PIPE,
#                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
#             p.communicate()
#             print("To configure NADEEF, please follow the following steps:")
#             print("1. Create a database entitled 'naadeef' in the postgres.")
#             postgress_username = raw_input("2. Inter your postgres username: ")
#             postgress_password = raw_input("3. Inter your postgres password: ")
#             nadeef_configuration_file = open("{}/NADEEF/nadeef.conf".format(TOOLS_FOLDER), "r")
#             nadeef_configuration = nadeef_configuration_file.read()
#             nadeef_configuration = re.sub("(database.username = )([\w\d]+)", "\g<1>{}".format(postgress_username),
#                                           nadeef_configuration, flags=re.IGNORECASE)
#             nadeef_configuration = re.sub("(database.password = )([\w\d]+)", "\g<1>{}".format(postgress_password),
#                                           nadeef_configuration, flags=re.IGNORECASE)
#             nadeef_configuration_file.close()
#             nadeef_configuration_file = open("{}/NADEEF/nadeef.conf".format(TOOLS_FOLDER), "w")
#             nadeef_configuration_file.write(nadeef_configuration)
#         print "{} is installed.".format(tool)
########################################


########################################
def read_csv_dataset(dataset_path):
    """
    The method reads a dataset from a csv file path.
    """
    dataset_file = open(dataset_path, "r")
    dataset_reader = csv.reader(dataset_file, delimiter=",")
    dataset_header = []
    dataset_matrix = []
    for i, row in enumerate(dataset_reader):
        row = [x.strip(" ") if x.lower() != "null" else "" for x in row]
        if i == 0:
            dataset_header = row
        else:
            dataset_matrix.append(row)
    return dataset_header, dataset_matrix


def write_csv_dataset(dataset_path, dataset_header, dataset_matrix):
    """
    The method writes a dataset to a csv file path.
    """
    dataset_file = open(dataset_path, "w")
    dataset_writer = csv.writer(dataset_file, delimiter=",")
    dataset_writer.writerow(dataset_header)
    for row in dataset_matrix:
        dataset_writer.writerow(row)
########################################


########################################
def run_dboost(dataset_path, dboost_parameters):
    """
    This method runs dBoost on a dataset.
    """
    tool_path="/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/civilizer_services/cleaning_service/"
    # command = ["./{}/dBoost/dboost/dboost-stdin.py".format(TOOLS_FOLDER), "-F", ",", dataset_path] + dboost_parameters

    command = [tool_path+"{}/dBoost/dboost/dboost-stdin.py".format(TOOLS_FOLDER), "-F", ",", dataset_path] + dboost_parameters
    p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    p.communicate()
    return_list = []
    tool_results_path = "dboost_results.csv"
    if os.path.exists(tool_results_path):
        tool_results_file = open(tool_results_path, "r")
        csv_reader = csv.reader(tool_results_file, delimiter=",")
        cell_visited_flag = {}
        for row in csv_reader:
            i = int(row[0])
            j = int(row[1])
            v = row[2]
            if (i, j) not in cell_visited_flag and i > 0:
                cell_visited_flag[(i, j)] = 1
                return_list.append([i, j, v])
        tool_results_file.close()
        os.remove(tool_results_path)
    return return_list


def run_nadeef(dataset_path, nadeef_parameters):
    """
    This method runs NADEEF on a dataset.
    """
    dataset_header, dataset_matrix = read_csv_dataset(dataset_path)
    temp_dataset_path = os.path.abspath("nadeef_temp_dataset.csv")
    new_header = [a + " varchar(20000)" for a in dataset_header]
    write_csv_dataset(temp_dataset_path, new_header, dataset_matrix)
    column_index = {a: dataset_header.index(a) for a in dataset_header}
    nadeef_clean_plan = {
        "source": {
            "type": "csv",
            "file": [temp_dataset_path]
        },
        "rule": nadeef_parameters
    }
    nadeef_clean_plan_path = "nadeef_clean_plan.json"
    nadeef_clean_plan_file = open(nadeef_clean_plan_path, "w")
    json.dump(nadeef_clean_plan, nadeef_clean_plan_file)
    nadeef_clean_plan_file.close()
    p = subprocess.Popen(["./nadeef.sh"], cwd="{}/NADEEF".format(TOOLS_FOLDER), stdout=subprocess.PIPE,
                         stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
    process_output, process_errors = p.communicate("load ../../nadeef_clean_plan.json\ndetect\nexit\n")
    os.remove(nadeef_clean_plan_path)
    return_list = []
    tool_results_path = re.findall("INFO: Export to (.*csv)", process_output)[0]
    tool_results_file = open(tool_results_path, "r")
    csv_reader = csv.reader(tool_results_file, delimiter=",")
    cell_visited_flag = {}
    for row in csv_reader:
        i = int(row[3])
        j = column_index[row[4]]
        v = row[5]
        if (i, j) not in cell_visited_flag:
            cell_visited_flag[(i, j)] = 1
            return_list.append([i, j, v])
    tool_results_file.close()
    os.remove(tool_results_path)
    os.remove(temp_dataset_path)
    return return_list


def run_openrefine(dataset_path, openrefine_parameters):
    """
    This method runs OpenRefine on a dataset.
    """
    dataset_header, dataset_matrix = read_csv_dataset(dataset_path)
    return_list = []
    cell_visited_flag = {}
    for i, row in enumerate(dataset_matrix):
        for j, pattern in openrefine_parameters:
            if not re.findall(pattern, row[j], re.IGNORECASE):
                if (i + 1, j) not in cell_visited_flag:
                    cell_visited_flag[(i + 1, j)] = 1
                    return_list.append([i + 1, j, row[j]])
    return return_list
########################################


########################################
def execute_cleaning(input_file_path, output_file_path):
    """
    This method runs the data cleaning tools on input datasets and saves the results into output locations.
    """
    input_dictionary = json.load(open(input_file_path, "r"))
    input_folder = input_dictionary["CSV"]["dir"]
    if input_dictionary["CSV"]["table"]:
        input_tables = input_dictionary["CSV"]["table"].split(";")
    else:
        input_tables = os.listdir(input_folder)
    output_dictionary = json.load(open(output_file_path, "r"))
    output_folder = output_dictionary["CSV"]["dir"]
    if not os.path.exists(output_folder):
        os.mkdir(output_folder)
    for table in input_tables:
        dataset_path = os.path.join(input_folder, table)
        results_list = run_dboost(dataset_path, ["--gaussian", "1", "--statistical", "1"])
        result_path = os.path.join(output_folder, table)
        write_csv_dataset(result_path, ["row", "column", "value"], results_list)
########################################


########################################
if __name__ == "__main__":
    input_file_path = "sources.json"
    output_file_path = "destination.json"
    execute_cleaning(input_file_path, output_file_path)
########################################
