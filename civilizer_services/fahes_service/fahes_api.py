import os
from subprocess import Popen
import json 
from os import listdir
import ctypes
from ctypes import c_char_p

global tool_loc
tool_loc = "./fahes/"
#tool_loc = "../civilizer_services/fahes_service/fahes/"

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


def execute_fahes(input_sources, output_location):
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
        print("File not found .. (", f_name, ")")
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
                                tab_ref = "csv//"+element[T]['dir']+"//"+Ts[i]
                                if element[T]['dir'].endswith('/'):
                                    tName = element[T]['dir']+Ts[i]
                                else:
                                    tName = element[T]['dir']+'/'+Ts[i]
                                callFahes(tab_ref, tName, output_dir)
                    else:
                        tables = element[T]['table'].split(';')
                        for i in tables:
                            tName = ""
                            tab_ref = "csv//"+element[T]['dir']+"//"+i
                            if element[T]['dir'].endswith('/'):
                                tName = element[T]['dir']+i
                            else:
                                tName = element[T]['dir']+'/'+i
                            callFahes(tab_ref, tName, output_dir)
                    
                else:
                    print ("Unsupported data type .. (", T, ")")
        
        

def callFahes(tab_ref, tab_full_name, output_dir):
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
    Fahes.execute(ref, tab_name, out_dir)
    

inputF = "sources.json"
outputF = "destination.json"
execute_fahes(inputF, outputF)


# filenames = listdir("/Users/qahtanaa/Documents/ErrorDetection/DataSets/MIT")
# for f in filenames:
#     print(f)
