import os
from subprocess import Popen
import json
from os import listdir
import ctypes
from ctypes import c_char_p
import pandas as pd
import csv
import numpy as np

from pandas.api.types import is_string_dtype

global tool_loc
# tool_loc = "./fahes/"
# tool_loc = "../civilizer_services/fahes_service/fahes/"
tool_loc = "/app/rest/services/fahes_service/fahes/"

########################################################################
#
#   New API (Oct. 2018)
#
########################################################################

'''
@author: giovnani@csail.mit.edu


TODO: this function will receive only params as input, which is a JSON with all the information stored there
The forllowing implementation is to be compieant with the old api
# def executeService(params):


'''
def executeService(source, out_path, params={}):
    dir_in = source['CSV']['dir']
    dir_out = out_path['CSV']['dir']
    
    dir_metadata = dir_in+"/metadata_fahes/"
    out_path['CSV']['dir']=dir_metadata # the output is redirected to the metadata folder
                                        #The purpose is twofold:
                                        #   - we do not change old API
                                        #   - we keep the meta-data
                                        # TODO: this will be a paramenter passed though the JSON input file
    if not os.path.exists(dir_metadata):
        os.makedirs(dir_metadata)

    execute_fahes(source, out_path) # not passing dir_metadata but the json for keeping the old API

    # todo with param, define which DMVs to apply
    transform_dmv_to_null(dir_in, dir_metadata, dir_out, params)


'''
Iterate through the files in the input directory
'''
def transform_dmv_to_null(dir_in, dir_metadata, dir_out, params={}):
    files_in = os.listdir(dir_in)
    
    # only .CSV supported, check extension:
    files_in = list(filter(lambda x: len(x.split(".csv"))>1,files_in))

    # for each file apply the tranformation
    for file in files_in:
        file_in_path = dir_in + file
        metadata_file_path = dir_metadata + "DMV_" + file # DMV_ is the prefix added by Fahes
        file_out_path = dir_out + file

        # apply the transformation (table is a pandas dataframe):
        table = transformSingleFile(file_in_path, metadata_file_path)

        table.to_csv(file_out_path,
                    sep=',',index=False,
                    quoting = csv.QUOTE_NONNUMERIC,
                    header=True
                    )


'''
A DMVs is a value that is equivalent to null, but for some reason it is not null.
E.g.: phone_number: 999-9999999
E.g.: age: -1
E.g.: name: ?

Fahes indicates which values for what attribute should be replaced with a null value.

This function allpy this transformation
'''


def transformSingleFile(file_in_path, metadata_file_path):
    table = pd.read_csv(file_in_path, encoding="ISO-8859-1")

    statinfo = os.stat(metadata_file_path)
    if (statinfo.st_size == 0):  ## if == 0 : do not read
        return table

    df_dmv = pd.read_csv(metadata_file_path, encoding="ISO-8859-1")

    dmv_attributes = set(df_dmv[" attribute name"].values)

    for attr in dmv_attributes:
        df_dmv_ = df_dmv[df_dmv[" attribute name"] == attr]
        if table[attr].dtypes == np.int:
            tt = set(df_dmv_[df_dmv[" attribute name"] == attr][" DMV"].values)

            num_null_fahes = sum(list(df_dmv_[df_dmv[" attribute name"] == attr][" frequency"].values))
            num_null_before = table[attr].isnull().values.sum()

            table[attr] = table[attr].astype(str)
            for i, t in enumerate(tt):
                t_string = str(t).split(".")[0]
                table[attr] = table[attr].replace(t_string, np.NaN)

            num_null_after = table[attr].isnull().values.sum()
            if (num_null_fahes - (num_null_after - num_null_before)) > 0:
                raise ValueError('Not all the nulls have been converted correctly.')
        else:
            df_dmv_.loc[:, (" DMV")].astype(table[attr].dtypes)
            tt = set(df_dmv_[df_dmv[" attribute name"] == attr][" DMV"].values)

            num_null_fahes = sum(list(df_dmv_[df_dmv[" attribute name"] == attr][" frequency"].values))

            num_null_before = table[attr].isnull().values.sum()
            for t in tt:
                table[attr] = [(s.strip() if isinstance(s, str) else s) for s in table[attr].values]
                table[attr] = table[attr].replace(t, np.NaN)
            num_null_after = table[attr].isnull().values.sum()
            if (int(num_null_fahes) - (num_null_after - num_null_before)) > 0:
                raise ValueError(
                    'Not all the nulls have been converted correctly.\nCheck "transformSingleFile()" in Fahes API')
    return table

# def transformSingleFile(file_in_path, metadata_file_path):
#     table = pd.read_csv(file_in_path, encoding = "ISO-8859-1") # pandas dataframe 
    
#     # some file may be empty, i.e., w/o transofmations
#     # TODO: Fahes should not retun such files
#     statinfo = os.stat(metadata_file_path)
#     if(statinfo.st_size==0): ## if == 0 : do not read and return w/o tranformations
#         return table

#     df_dmv = pd.read_csv(metadata_file_path, encoding = "ISO-8859-1")
    

#     dmv_attributes = set(df_dmv[" attribute name"].values) # the names of the attirbutes that have DMVs

#     for attr in dmv_attributes:
#         df_dmv_ = df_dmv[df_dmv[" attribute name"]==attr] # select the values 
#         df_dmv_[" DMV"] = df_dmv_[" DMV"].astype(table[attr].dtypes) # Fahes outputs are strings, so we need to ensure
#                                                                      # that the type is matained
#         # set of values to change to null
#         tt = set(df_dmv_[df_dmv[" attribute name"]==attr][" DMV"].values)
#         #a = table[attr].isnull().values.sum()
#         for t in tt:
#             table[attr] = table[attr].replace(t, np.NaN)
#         #b = table[attr].isnull().values.sum()
#         #print(b-a)
#     return table


########################################################################
#
#   Old APIs
#
########################################################################


def read_csv_directory(dir_name):
    csv_tables_names = []
    data_path = os.path.abspath(dir_name);
    if csv_tables_names:
        for i in range(len(csv_tables_names)):
            csv_datafreames.remove(csv_tables_names[0])     
    file_extension = '.csv'
    try:
        filenames = listdir(data_path)
    except Exception as e:
        if hasattr(e, 'message'):
            print ("Error occured (", a, ")")
        else:
            print ("Error occured (", e, ")")        
        return None
    return [filename for filename in filenames if filename.endswith( file_extension ) ]


def execute_fahes(source, out_path, debug=0):
    out_dir = ""

    # for EL in out_path:
    #     if EL.lower() == 'csv':
    #         out_dir = out_path[EL]['dir']

    out_dir = out_path['CSV']['dir']

    output_dir = ""
    if out_dir:
        output_dir = os.path.abspath(out_dir)
        if not output_dir:
            print("Cannot locate absolute location of output directory")
    else:
        print("Cannot locate output directory")
        return

    myDir = source['CSV']['dir']
    myTables = source['CSV']['table']

    if not (myTables):
        Ts = read_csv_directory(myDir)
        if Ts:
            for i in range(len(Ts)):
                tName = ""
                tab_ref = "csv::" + myDir + "::" + Ts[i]
                if myDir.endswith('/'):
                    tName = myDir + Ts[i]
                else:
                    tName = myDir + '/' + Ts[i]
                callFahes(tab_ref, tName, output_dir, debug)
        else:
            tables = myTables.split(';')
            for i in tables:
                tName = ""
                tab_ref = "csv::" + myDir + "::" + i
                if myDir.endswith('/'):
                    tName = myDir + i
                else:
                    tName = myDir + '/' + i
                callFahes(tab_ref, tName, output_dir, debug)



def execute_fahes_files(input_sources, output_location, debug = 0):
    out_dir = ""
    try:
        with open(output_location) as output_loc:
            try:
                # print(os.path.abspath(f_name))    
                out_path = json.load(output_loc)
                for EL in out_path:
                    if EL.lower() == 'csv':
                        out_dir = out_path[EL]['dir']
            except Exception as e:
                if hasattr(e, 'message'):
                    print("Cannot read json file .. (", e.message, ")")
                else:
                    print("Cannot read json file .. (", e, ")")
                return None
    except:
        print("File not found .. (", output_location, ")")
        return None

    output_dir = ""
    # print("Out directory (", out_dir, ")")
    if out_dir:
        output_dir = os.path.abspath(out_dir)
        if not output_dir:
            print("Cannot locate absolute location of output directory")
    else:
        print("Cannot locate output directory")
        return 
    
    if not os.path.exists(output_dir):
            os.makedirs(output_dir)
    sources = input_sources
    files = sources.split(';')
    sources_list = []
    if sources_list:
        for i in range(len(sources_list)):
            sources_list.remove(sources_list[0])
    for f_name in files:
        if f_name:
            # print(f_name)
            try:
                with open(f_name) as data_file:
                    try:
                        # print(os.path.abspath(f_name))    
                        data = json.load(data_file)
                        sources_list.append(data)
                    except:
                        print("Cannot read json file .. (", f_name, ")")
                        return None
            except:
                print("File not found .. (", f_name, ")")
                continue
    tName = ""


    for element in sources_list:
        if element:
            for T in element:
                if T.lower() == 'csv':
                    if not (element[T]['table']):
                        Ts = read_csv_directory(element[T]['dir'])
                        if Ts:
                            for i in range(len(Ts)):
                                tName = ""
                                tab_ref = "csv::"+element[T]['dir']+"::"+Ts[i]
                                if element[T]['dir'].endswith('/'):
                                    tName = element[T]['dir']+Ts[i]
                                else:
                                    tName = element[T]['dir']+'/'+Ts[i]
                                callFahes(tab_ref, tName, output_dir, debug)
                    else:
                        tables = element[T]['table'].split(';')
                        for i in tables:
                            tName = ""
                            tab_ref = "csv::"+element[T]['dir']+"::"+i
                            if element[T]['dir'].endswith('/'):
                                tName = element[T]['dir']+i
                            else:
                                tName = element[T]['dir']+'/'+i
                            callFahes(tab_ref, tName, output_dir, debug)
                    
                else:
                    print ("Unsupported data type .. (", T, ")")
        
        

def callFahes(tab_ref, tab_full_name, output_dir, debug):
    global tool_loc
    g_path = os.path.abspath(tool_loc)
    path = ""
    if not g_path.endswith('/'):
        path = g_path + "/libFahes.so"
    else:
        path = g_path + "libFahes.so"

    ref = c_char_p(tab_ref.encode('utf-8'))
    tab_name = c_char_p(tab_full_name.encode('utf-8'))
    out_dir = c_char_p(output_dir.encode('utf-8'))
    Fahes=ctypes.cdll.LoadLibrary(path)
    Fahes.execute(ref, tab_name, out_dir, debug)



'''
import os
if not os.path.exists(directory):
    os.makedirs(directory)


- create a folder "metadata"
For file in input:
    DMV (vedi script)
'''
