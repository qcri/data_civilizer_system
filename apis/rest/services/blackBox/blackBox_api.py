import os
import subprocess

def log_text(text):
    log_path = '/app/'
    file_name = log_path + 'log.txt'
    f = open(file_name, "a+")
    f.write(text)
    f.write('\n')
    f.close

def executeService(parameters, inputs):
    from civilizer import getOutputDirectory


    from shutil import copyfile
    log_text('BlackBox2')
    log_text(parameters["param4"])


    filelist = []

    # This command create the tmp dir, where this service can write the files
    try:
        out_dir_path = getOutputDirectory(parameters)
    except OSError as err:
        return {"error": "OSError: {0}".format(err)}

    if not os.path.exists(out_dir_path):
        os.makedirs(out_dir_path)

    # these are the input files passed from civilizer.py
    '''
    Here we just copy the files from the input dir to the output dir.
    The logic of the operator should go here, e.g., calling the doThings() function
    '''
    for input in inputs:
        if 'civilizer.dataCollection.filelist' in input:
            filelist.extend(input['civilizer.dataCollection.filelist'])
            for file in input['civilizer.dataCollection.filelist']:
            	doThings(file)
                file_tmp = out_dir_path + file.split("/")[-1]
                copyfile(file, file_tmp)
                filelist.append(file_tmp)

    # these are the files passed to the next operator
    output = {'civilizer.dataCollection.filelist': filelist}
    return output


def doThings(file):
	pass