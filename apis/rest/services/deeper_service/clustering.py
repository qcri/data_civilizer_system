import networkx as nx
import pandas as pd
import csv


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




