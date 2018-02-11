import subprocess
import networkx as nx
import pandas as pd
import csv

#########################################
def write_clustered_csv(c_data, matches_file):
    keys = c_data[0].keys()
    with open(matches_file, 'w') as output_file:
        dict_writer = csv.DictWriter(output_file, keys)
        dict_writer.writeheader()
        dict_writer.writerows(c_data)

def readCsv(csvFile, column, data):
    with open(csvFile, "r", encoding='utf-8', errors='ignore') as infile:
        reader = csv.DictReader(infile, delimiter=',')
        for row in reader:
            data[row[column]] = row


def createClusters(table1, table2, prediction_file, matches_file):
    # table1="Amazon_full.csv"
    data_t1={}
    readCsv(table1, 'id', data_t1)
    # print(data_t1['b000ayznhg'])

    # table2="GoogleProducts_full.csv"
    data_t2={}
    readCsv(table2, 'id', data_t2)
    # print(data_t2['http://www.google.com/base/feeds/snippets/1943468973342706865'])

    cid = 0
    cluster_id_att = "cluster_id"
    clustered_data = []

    df=pd.read_csv(prediction_file, sep=',', header=None)
    data = df.values

    G = nx.Graph()
    G.add_edges_from(data)
    for connected_component in nx.connected_components(G):
        # print(connected_component)
        cluster_id_value = "cluster_id" + "_" + str(cid)
        print("\n" + cluster_id_value)
        new_cluster = {cluster_id_att: cluster_id_value}
        for item in connected_component:
            if item in data_t1:
                new_row = dict(list(data_t1[item].items()) + list(new_cluster.items()))
                clustered_data.append(new_row)
                print(new_row)
            elif item in data_t2:
                new_row = dict(list(data_t2[item].items()) + list(new_cluster.items()))
                clustered_data.append(new_row)
                print(new_row)
        cid = cid + 1

    write_clustered_csv(clustered_data, matches_file)
########################################
def execute_deeper(source, table1, table2, number_of_pairs, destination, predictionsFileName):
    """
    This method runs deeper on a dataset.
    """
    table1_path = ""
    table2_path = ""
    predictions_file_path = ""
    pred_pairs_path = ""
    threshold_path = ""

    if source.endswith("/"):
        table1_path = source + table1
        table2_path = source + table2
        threshold_path = source + "threshold.txt"
    else:
        table1_path = source + "/" + table1
        table2_path = source + "/" + table2
        threshold_path = source + "/" + "threshold.txt"

    if destination.endswith("/"):
        predictions_file_path = destination + predictionsFileName
        pred_pairs_path = destination + "pred_pairs_" + number_of_pairs + ".csv"
    else:
        predictions_file_path = destination + "/" + predictionsFileName
        pred_pairs_path = destination + "/" + "pred_pairs_" + number_of_pairs + ".csv"


    params=[source,
            table1_path,
            table2_path,
            "4",
            pred_pairs_path,
            number_of_pairs,
            predictions_file_path,
            threshold_path,
            "1",
            destination
            ]

    print("number of pairs is " + number_of_pairs)
    print("threshold file is " + threshold_path)

    # params=["/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/Amazon-GoogleProducts",
    #         "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/Amazon_full.csv",
    #         "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/GoogleProducts_full.csv",
    #         "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/pred_pairs_No.csv",
    #         "10000",
    #         "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/output/matches.csv",
    #         "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/threshold.txt",
    #         "/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/storage/data_sets/deeper/output"
    #         ]

    # tool_path="/Users/emansour/elab/DAGroup/DataCivilizer/github/data_civilizer_system/civilizer_services/deeper_service/DeepER-Lite/"
    tool_path = "/app/rest/services/deeper_service/DeepER-Lite/"
    # tool_path = "/app/DeepER-Lite/"
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

    print("create Clusters")

    prediction_file = predictions_file_path.replace(".csv", "_0.csv")

    createClusters(table1_path, table2_path, prediction_file, predictions_file_path)





