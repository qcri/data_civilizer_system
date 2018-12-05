import subprocess
import networkx as nx
import pandas as pd
import csv
import py_entitymatching  # see Magellan
import os

import blocking_utils
from process_dataset_sim import split_dataset_by_ratio
import deeper_lite_sim as deeper

########################################################################
#
#   New API (Oct. 2018)
#
########################################################################

'''
@author: giovnani@csail.mit.edu

example of parameter to pass
{
    "dataset_name": "Fodors_Zagat",
    "split_ratio": "",
    "Fodors_Zagat": {
        "dataset_folder_path": "/Users/gio/Workspace/DC/deeper-lite/python/BenchmarkDatasets/Fodors_Zagat/",
        "ltable_file_name": "fodors.csv",
        "rtable_file_name": "zagats.csv",
        "golden_label_file_name": "matches_fodors_zagats.csv",
        "attribute_list" : {"l" : ['name', 'addr', 'city', 'phone']
                            "r" : ['name', 'addr', 'city', 'phone']},
        "blocking_key" : {"l" : 'name', "r" : 'name',}
    },
    "out_file_path":"./"
}


'''


def executeService(params={}):
    dataset_name = params["dataset_name"]

    #
    # This is a default blocking. (overlap blocking from Magellan, a.k.a. Standard Blocking)
    # Please refer to the documentation of DeepER (and Magellan) for custom blocking
    # blocking generates the file "candset_ids_only.csv"
    #
    #

    l_file_name = params[dataset_name]["dataset_folder_path"] + params[dataset_name]["ltable_file_name"]
    r_file_name = params[dataset_name]["dataset_folder_path"] + params[dataset_name]["rtable_file_name"]
    A = py_entitymatching.read_csv_metadata(l_file_name, key="id", encoding='utf-8')
    B = py_entitymatching.read_csv_metadata(r_file_name, key="id", encoding='utf-8')

    ob = py_entitymatching.OverlapBlocker()
    # No misses
    candset_df = ob.block_tables(A, B,
                                 params[dataset_name]["blocking_key"]["l"],
                                 params[dataset_name]["blocking_key"]["r"],
                                 l_output_attrs=params[dataset_name]["attribute_list"]["l"],
                                 r_output_attrs=params[dataset_name]["attribute_list"]["r"],
                                 overlap_size=1,
                                 show_progress=False)

    candset_with_labels_df = blocking_utils.add_labels_to_candset(dataset_name, candset_df, False)
    blocking_utils.save_candset_compressed(dataset_name, candset_with_labels_df, "candset.pkl.compress")
    blocking_utils.save_candset_ids_only(dataset_name, candset_with_labels_df)

    #
    # Split in train-validation-test
    #
    split_ratio = [0.3, 0.2, 0.5]
    if params["split_ratio"] is not "":
        split_ratio = params["split_ratio"]

    folder_path = params[dataset_name]["dataset_folder_path"]
    candset_ids_file_name = "candset_ids_only.csv"
    split_dataset_by_ratio(folder_path,
                           candset_ids_file_name,
                           split_ratio=split_ratio,
                           label_field_name='gold',
                           random_state=12345,
                           train_file_name="train.csv",
                           validation_file_name="validation.csv",
                           test_file_name="test.csv")

    # train the model
    deeper.train(dataset_name,
                 "train.csv",
                 "validation.csv",
                 deeper.get_deeper_lite_model_sim)

    # predict the resutls (test)
    deeper.test(dataset_name,
                "test.csv",
                params["out_file_path"]+"test_predictions.csv",
                deeper.get_deeper_lite_model_sim)

    df_a = pd.read_csv(l_file_name, encoding='utf8')
    df_b = pd.read_csv(r_file_name, encoding='utf8')
    df_pred = pd.read_csv(params[dataset_name]["dataset_folder_path"] + "candset_ids_only.csv", encoding='utf8')

    id1 = set(df_a.id)
    id2 = set(df_b.id)

    df_matches = df_pred[df_pred.gold == 1]

    df_a['cluster_id'] = df_a.id
    df_b['cluster_id'] = df_b.id

    G = nx.Graph()
    G.add_edges_from(df_matches[["ltable_id", "rtable_id"]].values)

    cluster = -1
    cc = 0
    for component in nx.connected_components(G):
        cluster += 1
        for c in component:
            if c in id1:
                df_a.loc[df_a.id == c, 'cluster_id'] = cluster
            else:
                df_b.loc[df_b.id == c, 'cluster_id'] = cluster

    if not os.path.exists(params["out_file_path"]):
        os.makedirs(params["out_file_path"])

    file_out_path_a = params["out_file_path"] + params[dataset_name]["ltable_file_name"]
    file_out_path_b = params["out_file_path"] + params[dataset_name]["rtable_file_name"]

    df_a.to_csv(file_out_path_a,
                sep=',', index=False,
                quoting=csv.QUOTE_NONNUMERIC,
                header=True
                )

    df_a.to_csv(file_out_path_b,
                sep=',', index=False,
                quoting=csv.QUOTE_NONNUMERIC,
                header=True
                )