# GiG.

# Add local directory to path to allow imports when invoked directly or as part of Data Civilizer
import os
import sys
sys.path.append(os.path.realpath(os.path.dirname(__file__)))

import torch
import torch.nn as nn
import torch.utils.data as Data
import torch.optim as optim

from sklearn.metrics import f1_score
import random
import numpy as np
import pandas as pd

import py_entitymatching
import blocking_utils
from process_dataset_sim import split_dataset_by_ratio
import networkx as nx
import csv

import process_dataset_sim as process_dataset

# DL Specific configs
BATCH_SIZE = 16
MAX_EPOCHS = 32
LEARNING_RATE = 0.001
BETAS = (0.9, 0.99)
EPSILON = 1e-9
RANDOM_STATE = 12345
HIDDEN_X = 2
MODEL_FILE_NAME = "best_validation_model_params.torch"


def get_deeper_lite_model_sim(num_attributes):
    # If there are K input attributes, Deeper Lite  has 2K features : 1 each for cosine distance and normed abs distance
    # Hidden_X is a multiplicative factor controlling the size of hidden layer.
    deeper_lite_model = nn.Sequential(
        nn.Linear(2 * num_attributes, HIDDEN_X * num_attributes),
        nn.ReLU(),
        nn.Linear(HIDDEN_X * num_attributes, HIDDEN_X * num_attributes),
        nn.ReLU(),
        nn.Linear(HIDDEN_X * num_attributes, HIDDEN_X * num_attributes),
        nn.ReLU(),
        nn.Linear(HIDDEN_X * num_attributes, 2),
    )

    return deeper_lite_model


# Assumes that the train and validation files are in the same folder as dataset_name
def train(dataset_name, train_file_name, validation_file_name, model_fn):
    train_features, train_labels = process_dataset.get_features_and_labels(dataset_name, train_file_name)
    validation_features, validation_labels = process_dataset.get_features_and_labels(dataset_name, validation_file_name)

    # Hack: Assumes that for deeper lite num_features = 2 * num_attributes
    num_attributes = int(train_features.shape[1] / 2)
    model = model_fn(num_attributes)

    train_dataset = Data.TensorDataset(train_features, train_labels)
    # Allows us to read the dataset in batches
    training_loader = Data.DataLoader(dataset=train_dataset, batch_size=BATCH_SIZE, shuffle=True)

    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE, betas=BETAS, eps=EPSILON)
    criterion = nn.CrossEntropyLoss()

    # For reproducibility
    random.seed(RANDOM_STATE)
    np.random.seed(RANDOM_STATE)
    torch.manual_seed(RANDOM_STATE)

    best_validation_f1_score = 0.0
    best_model_so_far = None
    model_file_name_path = os.path.join(process_dataset.get_folder_to_persist_model(dataset_name), MODEL_FILE_NAME)

    for epoch in range(MAX_EPOCHS):
        for batch_idx, (train_features, train_labels) in enumerate(training_loader):
            optimizer.zero_grad()
            output = model(train_features)
            loss = criterion(output, train_labels)
            loss.backward()
            optimizer.step()

        training_f1_score = compute_scores(output, train_labels)
        with torch.no_grad():
            validation_output = model(validation_features)
            validation_f1_score = compute_scores(validation_output, validation_labels)
            if validation_f1_score > best_validation_f1_score:
                best_model_so_far = model.state_dict()
                best_validation_f1_score = validation_f1_score

    torch.save(best_model_so_far, model_file_name_path)
    print("Curr Val F1, Best Val F1 " + str(validation_f1_score) + " " + str(best_validation_f1_score))
    return best_model_so_far


def test(dataset_name, test_file_name, test_output_file_name, model_fn):
    test_features, test_labels = process_dataset.get_features_and_labels(dataset_name, test_file_name)
    # Hack: Assumes that for deeper lite num_features = 2 * num_attributes
    num_attributes = test_features.shape[1] / 2
    model = model_fn(num_attributes)

    folder_path = process_dataset.get_folder_to_persist_model(dataset_name)
    model_file_name_path = os.path.join(folder_path, MODEL_FILE_NAME)
    model.load_state_dict(torch.load(model_file_name_path))
    model.eval()

    predictions = model(test_features)
    # Uncomment the following lines to get the score
    # testing_f1_score = compute_scores(predictions, test_labels
    # print "Testing F1 ", testing_f1_score

    prediction_as_numpy = torch.max(predictions, 1)[1].data.numpy()

    # Store output
    test_df = pd.read_csv(os.path.join(folder_path, test_file_name), encoding="utf-8")
    test_df["gold"] = prediction_as_numpy
    test_df.to_csv(os.path.join(folder_path, test_output_file_name), encoding="utf8", index=False)


def predict(dataset_name, predict_file_name, predict_output_file_name, model_fn):
    test_features = process_dataset.get_features_only(dataset_name, predict_file_name)
    # Hack: Assumes that for deeper lite num_features = 2 * num_attributes
    num_attributes = int(test_features.shape[1] / 2)
    model = model_fn(num_attributes)

    folder_path = process_dataset.get_folder_to_persist_model(dataset_name)
    model_file_name_path = os.path.join(folder_path, MODEL_FILE_NAME)
    model.load_state_dict(torch.load(model_file_name_path))
    model.eval()

    predictions = model(test_features)
    # Uncomment the following lines to get the score
    # testing_f1_score = compute_scores(predictions, test_labels)
    # print "Testing F1 ", testing_f1_score

    prediction_as_numpy = torch.max(predictions, 1)[1].data.numpy()

    # Store output
    test_df = pd.read_csv(os.path.join(folder_path, predict_file_name), encoding="utf-8")
    test_df["gold"] = prediction_as_numpy
    test_df.to_csv(os.path.join(folder_path, predict_output_file_name), encoding="utf8", index=False)


def compute_scores(predicted, actual):
    # Convert from cross entropy output to actual 0/1 predictions
    predicted = torch.max(predicted, 1)[1].data

    # Convert to numpy format
    predicted_numpy = predicted.numpy()
    actual_numpy = actual.numpy()

    # Print performance measures
    return f1_score(actual_numpy, predicted_numpy)


'''
This is the service for training the DeepER Model
Required input:
- dataset_a
- dataset_b
- labeled_data (labeled_ids_only.csv)
'''


def executeServiceTrain(params={}):
    dataset_name = params["dataset_name"]

    # split_ratio = [0.7, 0.3, 0.0]
    split_ratio = [0.8, 0.2, 0.0]
    # if params["split_ratio"] is not "":
    #     split_ratio = params["split_ratio"]

    folder_path = params[dataset_name]["dataset_folder_path"]
    candset_ids_file_name = "labeled_ids_only.csv"
    split_dataset_by_ratio(folder_path,
                           candset_ids_file_name,
                           split_ratio=split_ratio,
                           label_field_name='gold',
                           random_state=12345,
                           train_file_name="train.csv",
                           validation_file_name="validation.csv",
                           test_file_name="test.csv")  # test should be empty

    train(dataset_name,
          "train.csv",
          "validation.csv",
          get_deeper_lite_model_sim)


'''
This is the service for executing the prediction, given the trained DeepER model
Required input:
- dataset_a
- dataset_b
Optional:
- predict.csv (candidates pairs generated with blocking)
'''


def executeServicePredict(params={}):
    dataset_name = params["dataset_name"]

    #
    # This is a default blocking. (overlap blocking from Magellan, a.k.a. Standard Blocking)
    # Please refer to the documentation of DeepER (and Magellan) for custom blocking
    # blocking generates the file "predict.csv"
    #
    #
    l_file_name = params[dataset_name]["dataset_folder_path"] + params[dataset_name]["ltable_file_name"]
    r_file_name = params[dataset_name]["dataset_folder_path"] + params[dataset_name]["rtable_file_name"]

    generateCandidates = True if params[dataset_name]["candidates_file"] is "" else False

    if generateCandidates:
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

        blocking_utils.save_candset_compressed(dataset_name, candset_df, "candset.pkl.compress")

        folder_path = process_dataset.get_folder_to_persist_model(dataset_name)

        candset_df["gold"] = 0
        candset_df_id_oly = candset_df[["ltable_id", "rtable_id", "gold"]]

        candset_df_id_oly.to_csv(folder_path + "predict.csv",
                                 sep=',', index=False,
                                 # quoting=csv.QUOTE_NONNUMERIC,
                                 header=True
                                 )

    # predict_file_name, predict_output_file_name
    predict(dataset_name,
            "predict.csv",  # this file has to be provided (before: test.csv)
            "test_predictions.csv",  # output file
            get_deeper_lite_model_sim)

    df_a = pd.read_csv(l_file_name, encoding='utf8')
    df_b = pd.read_csv(r_file_name, encoding='utf8')
    df_pred = pd.read_csv(params[dataset_name]["dataset_folder_path"] + "labeled_ids_only.csv", encoding='utf8')

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
                encoding="utf8",
                quoting=csv.QUOTE_NONNUMERIC,
                header=True
                )

    df_b.to_csv(file_out_path_b,
                sep=',', index=False,
                encoding="utf8",
                quoting=csv.QUOTE_NONNUMERIC,
                header=True
                )


if __name__ == "__main__":
    params = {
        "dataset_name": "movies",
        "movies": {
            "dataset_folder_path": "/Users/gio/Workspace/DC/deeper-lite/python/BenchmarkDatasets/movies/",
            "ltable_file_name": "dataset_a.csv",
            "rtable_file_name": "dataset_b.csv",
            "blocking_key": {"l": "movie_name", "r": "movie_name"},
            "candidates_file": "predict.csv",
            "attribute_list": {"l": ['genre', 'year', 'actors', 'movie_name', 'directors', 'duration'],
                               "r": ['genre', 'year', 'actors', 'movie_name', 'directors', 'duration']}
        },
        "out_file_path": "/Users/gio/Workspace/DC/deeper-lite/python/BenchmarkDatasets/movies/out2/"
    }

    # executeService(params)
    executeServiceTrain(params)
    executeServicePredict(params)

    # train("Fodors_Zagat", "train.csv", "validation.csv", get_deeper_lite_model_sim)
    # test("Fodors_Zagat", "test.csv", "test_predictions.csv", get_deeper_lite_model_sim)
    #
    # train("Cora", "train.csv", "validation.csv", get_deeper_lite_model_sim)
    # test("Cora", "test.csv", "test_predictions.csv", get_deeper_lite_model_sim)
    #
    # train("DBLP_ACM", "train.csv", "validation.csv", get_deeper_lite_model_sim)
    # test("DBLP_ACM", "test.csv", "test_predictions.csv", get_deeper_lite_model_sim)
    #
    # train("DBLP_Scholar", "train.csv", "validation.csv", get_deeper_lite_model_sim)
    # test("DBLP_Scholar", "test.csv", "test_predictions.csv", get_deeper_lite_model_sim)
