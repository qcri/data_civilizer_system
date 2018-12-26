import os
import json
import ctypes
from ctypes import c_double
from ctypes import c_char_p
from sys import platform
import shutil

SELF_DIR_PATH = os.path.dirname(os.path.realpath(__file__))
if platform == "darwin":
    DYNAMIC_LIB = "libpkduck.dylib"
else:
    DYNAMIC_LIB = "libpkduck.so"

# columns are in the format of 1,3,4#5#2#4,7, meaning that 
# the 1st, 3rd and 4th columns of the first table are selected 
# the 5th column of the second table is selected, etc. 
# tau is the similarity threshold, between 0 and 1 (usually greater than 0.7)

def execute_pkduck(input_json, output_json, columns, tau):

    i = input_json['CSV']['dir'];
    input_dir = c_char_p(i.encode('utf-8'))
    o = output_json['CSV']['dir'] ;
    output_dir = c_char_p(o.encode('utf-8'))

    columns = c_char_p(columns.encode('utf-8'))

    tau = c_double(tau)
    pkduck = ctypes.cdll.LoadLibrary(SELF_DIR_PATH + "/code/" + DYNAMIC_LIB)
    pkduck.execute(input_dir, output_dir, columns, tau)


def executeService_params(params, inputs):
    from civilizer import getOutputDirectory, parseQuery

    filelist = inputs[0]['civilizer.dataCollection.filelist']

    copylist = filelist.copy()

    indexes = []
    pkfiles = []

    try:
        for query in params['civilizer.PKDuck.columnSelect']:
            filepath, table_name, columns = parseQuery(filelist, query, True)
            if filepath in pkfiles:
                raise NameError("Duplicate table name in query list, '{0}'.".format(table_name))
            pkfiles.append(filepath)
            indexes.append(columns)
            copylist.remove(filepath)

        output_dir = getOutputDirectory(params)
        for filepath in copylist:
            shutil.copy(filepath, output_dir)

        metadata_dir = getOutputDirectory(params)

    except (NameError, OSError, SyntaxError) as err:
        return { "error": "{0}: {1}".format(type(err).__name__, err) }

    output = {
        'civilizer.PKDuck.auxiliary': metadata_dir + 'auxiliary_pkduck.csv',
        'civilizer.PKDuck.simstring': metadata_dir + 'simstring_pkduck.csv',
        'civilizer.dataCollection.filelist': [output_dir + os.path.basename(x) for x in filelist]
    }

    cfiles = len(pkfiles)

    input_files = (c_char_p * cfiles)()
    input_files[:] = [x.encode('utf-8') for x in pkfiles];

    output_dir = c_char_p(output_dir.encode('utf-8'))

    columns = c_char_p('#'.join(indexes).encode('utf-8'))

    tau = c_double(params['civilizer.PKDuck.tau'])

    metadata_dir = c_char_p(metadata_dir.encode('utf-8'))

    pkduck = ctypes.cdll.LoadLibrary(SELF_DIR_PATH + "/code/" + DYNAMIC_LIB)
    pkduck.execute_params(cfiles, input_files, output_dir, columns, tau, metadata_dir)

    return output


def execute_pkduck_file(input_json_file, output_json_file, columns, tau):
    with open(input_json_file, 'r') as f:
        input_json = json.load(f)
    with open(output_json_file, 'r') as f:
        output_json = json.load(f)

    i = input_json['CSV']['dir'];
    input_dir = c_char_p(i.encode('utf-8'))
    o = output_json['CSV']['dir'] ;
    output_dir = c_char_p(o.encode('utf-8'))

    columns = c_char_p(columns.encode('utf-8'))

    tau = c_double(tau)
    pkduck = ctypes.cdll.LoadLibrary(SELF_DIR_PATH + "/code/" + DYNAMIC_LIB)
    pkduck.execute(input_dir, output_dir, columns, tau)

if __name__ == '__main__':
    columns = "1#2"
    execute_pkduck_file("input.json", "output.json", columns, 0.8)
