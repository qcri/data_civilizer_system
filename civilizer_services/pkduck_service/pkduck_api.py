import os
import json
import ctypes
from ctypes import c_double
from ctypes import c_char_p

SELF_DIR_PATH = os.path.dirname(os.path.realpath(__file__))

# columns are in the format of 1,3,4#5#2#4,7, meaning that 
# the 1st, 3rd and 4th columns of the first table are selected 
# the 5th column of the second table is selected, etc. 
# tau is the similarity threshold, between 0 and 1 (usually greater than 0.7) 
def execute_pkduck(input_json_file, output_json_file, columns, tau):
    with open(input_json_file, 'r') as f:
        input_json = json.load(f)
    with open(output_json_file, 'r') as f:
        output_json = json.load(f)

    input_dir = c_char_p(input_json['CSV']['dir'] + "/")
    output_dir = c_char_p(output_json['CSV']['dir'] + "/")
    columns = c_char_p(columns)
    tau = c_double(tau)
    pkduck = ctypes.cdll.LoadLibrary(SELF_DIR_PATH + "/pkduck.dylib")
    pkduck.execute(input_dir, output_dir, columns, tau)

if __name__ == '__main__':
    columns = "12#11#8#7#1,2,7#10"
    execute_pkduck("input.json", "output.json", columns, 0.8)