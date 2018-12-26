from __future__ import print_function

import glob
import json
import os
import shutil
import subprocess
import tempfile
import uuid
import re

import csv
import numpy as np
import pandas as pd

SELF_DIR_PATH = os.path.dirname(os.path.realpath(__file__))
# STORAGE_PATH = SELF_DIR_PATH + '/../../storage/imputedb'
STORAGE_PATH = SELF_DIR_PATH + '/app/storage/imputedb'

IMPUTEDB_PATH = SELF_DIR_PATH + '/imputedb/imputedb'
DB_PATH = STORAGE_PATH + '/tmp.db'
INPUT_PATH = STORAGE_PATH + '/inputs'
OUTPUT_PATH = STORAGE_PATH + '/out.csv'

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
def executeService(source, out_path, q, r, params={}):
    dir_in = source['CSV']['dir']
    dir_out = out_path['CSV']['dir']
    
    dir_metadata = dir_in+"metadata_imputeBD/"

    out_path['CSV']['dir']=dir_metadata # The output is redirected to the metadata folder
                                        # TODO: this will be a paramenter passed though the JSON input file

    if not os.path.exists(dir_metadata):
        os.makedirs(dir_metadata)

    execute_imputedb(source, out_path, q, r)

    transform_null_to_imVal(dir_in+ source['CSV']['table']+".csv", dir_metadata+"out.csv", dir_out, params)


def transform_null_to_imVal(dir_in, dir_metadata, dir_out, params={}):
    data_in = pd.read_csv(dir_in, encoding='iso-8859-1')
    imputed_values = pd.read_csv(dir_metadata, encoding='iso-8859-1')
    cc = list(imputed_values.columns)
    cols = list(map(lambda c: c.split(".")[1], cc))
    imputed_values.columns= cols
    for c in cols:
        data_in[c] = imputed_values[c]

    data_in.to_csv(dir_out + (dir_in.split("/")[-1]),
                    sep=',',index=False,
                    quoting = csv.QUOTE_NONNUMERIC,
                    header=True
                    )


def getOutputDirectory(parameters):
    tmpdir = ""
    if 'civilizer.dataCollection.tmpdir' in parameters:
        tmpdir = parameters['civilizer.dataCollection.tmpdir']
    if not tmpdir:
        tmpdir = tempfile.gettempdir()
    output_dir = os.path.abspath(tmpdir + "/" + str(uuid.uuid4()))

    # Will raise an exception if output_dir already exists or cannot be created
    os.makedirs(output_dir)

    return output_dir + "/"


def getTableName(filelist, query):
    # Use regex to perform simplistic query parsing
    re_query = re.compile("^\\s*SELECT\\s*([^;]*\\S)\\s*FROM\\s*([^;]*\\S)\\s*;\\s*$", re.IGNORECASE)
    match = re_query.match(query)
    if not match:
        raise SyntaxError("Unrecognized column name query, '{0}'.".format(query))

    # Identify the CSV from filelist associated with the table name from query
    table_name = match.group(2)
    re_table = re.compile("/" + re.escape(table_name) + "\\.[Cc][Ss][Vv]$")
    filepath = [x for x in filelist if re_table.search(x)]
    if len(filepath) == 0:
        raise NameError("Unrecognized table name, '{0}'.".format(table_name))
    if len(filepath) > 1:
        raise NameError("Ambiguous table name, '{0}'.".format(table_name))

    return filepath[0], table_name


def executeService_params(params, inputs):
    filelist = inputs[0]['civilizer.dataCollection.filelist']

    copylist = filelist.copy()

    dbfiles = {}

    try:
        metadata_dir = getOutputDirectory(params)

        for query in params['civilizer.DataCleaning.Imputedb.Query']:
            filepath, table_name = getTableName(filelist, query)
            if filepath in dbfiles:
                raise NameError("Duplicate table name in query list, '{0}'.".format(table_name))
            dbfiles[filepath] = query
            copylist.remove(filepath)

        output_dir = getOutputDirectory(params)
        for filepath in copylist:
            shutil.copy(filepath, output_dir)

    except (NameError, OSError, SyntaxError) as err:
        return { "error": "{0}: {1}".format(type(err).__name__, err) }

    try:
        load_cmd = [IMPUTEDB_PATH, 'load', '--db', DB_PATH] + list(dbfiles.keys())
        subprocess.check_call(load_cmd)

        for filepath, query in dbfiles.items():
            query_cmd = [IMPUTEDB_PATH, 'query', '--db', DB_PATH, '--csv', '-c', query]
            with open(OUTPUT_PATH, 'w') as f:
                subprocess.check_call(query_cmd, stdout=f)

            transform_null_to_imVal(filepath, OUTPUT_PATH, output_dir, params)

            os.remove(OUTPUT_PATH)

    finally:
        for f in glob.glob(INPUT_PATH + '/*csv'):
            os.remove(f)
        if os.path.isdir(DB_PATH):
            shutil.rmtree(DB_PATH)

    output = {
        'civilizer.dataCollection.filelist': [output_dir + os.path.basename(x) for x in filelist]
    }

    return output


########################################################################
#
#   Old APIs
#
########################################################################

def execute_imputedb(src, dst, query, alpha):
    try:
        csv_paths = []
        if 'CSV' in src:
            csv_paths += get_csv_paths(src)
        if 'postgres' in src:
            csv_paths += get_postgres_paths(src)

        load_cmd = [IMPUTEDB_PATH, 'load', '--db', DB_PATH] + csv_paths
        subprocess.check_call(load_cmd)

        query_cmd = [IMPUTEDB_PATH, 'query', '--db', DB_PATH, '--csv', '-c', query]
        with open(OUTPUT_PATH, 'w') as f:
            subprocess.check_call(query_cmd, stdout=f)

        if 'CSV' in dst:
            put_csv_output(dst)
        if 'postgres' in dst:
            put_postgres_output(dst)

    finally:
        for f in glob.glob(INPUT_PATH + '/*csv'):
            os.remove(f)
        if os.path.isdir(DB_PATH):
            shutil.rmtree(DB_PATH)
        if os.path.isfile(OUTPUT_PATH):
            os.remove(OUTPUT_PATH)


def get_csv_paths(src):
    csv_dir = src['CSV']['dir'] + '/'
    csv_tables = src['CSV']['table']

    assert os.path.isdir(csv_dir)

    if csv_tables == '':
        return glob.glob(csv_dir + '/*.csv')
    else:
        csv_paths = []
        for table in csv_tables.split(';'):
            csv_fn = csv_dir + table + '.csv'
            assert os.path.isfile(csv_fn), '{} must be a CSV file'.format(csv_fn)
            csv_paths += [csv_fn]
        return csv_paths


def get_postgres_paths(src):
    csv_paths = []
    pg_db = src['postgres']['database']
    pg_tables = src['postgres']['table'].split(';')
    pg_user = src['postgres']['user']
    pg_password = src['postgres']['password']
    pg_host = src['postgres']['host']
    pg_port = src['postgres']['port']

    cmd = [
        'psql',
        '-d', str(pg_db),
        '-h', str(pg_host),
        '-p', str(pg_port),
        '-U', str(pg_user),
        '--no-align', '--field-separator=","'
    ]
    env = os.environ.copy()
    env['PGPASSWORD'] = pg_password

    for table in pg_tables:
        table_cmd = cmd + [
            '-c', 'select * from {}'.format(table)
        ]
        csv_fn = '{}/{}.csv'.format(INPUT_PATH, table)
        with open(csv_fn, 'w') as f:
            p = subprocess.Popen(table_cmd, env=env, stdout=f)
            p.wait()
            if p.returncode < 0:
                raise RuntimeError('psql failed with return code {}'\
                                   .format(p.returncode))
        csv_paths += [csv_fn]
    return csv_paths


def put_csv_output(dst):
    out_dir = dst['CSV']['dir']
    shutil.copy(OUTPUT_PATH, out_dir)


def put_postgres_output(dst):
    raise RuntimeError('Not implemented.')


def execute_imputedb_file(src_json, dst_json, query, alpha):
    with open(src_json, 'r') as f:
        src = json.load(f)
    with open(dst_json, 'r') as f:
        dst = json.load(f)

    try:
        csv_paths = []
        if 'CSV' in src:
            csv_paths += get_csv_paths(src)
        if 'postgres' in src:
            csv_paths += get_postgres_paths(src)

        load_cmd = [IMPUTEDB_PATH, 'load', '--db', DB_PATH] + csv_paths
        subprocess.check_call(load_cmd)

        query_cmd = [IMPUTEDB_PATH, 'query', '--db', DB_PATH, '--csv', '-c', query]
        with open(OUTPUT_PATH, 'w') as f:
            subprocess.check_call(query_cmd, stdout=f)

        if 'CSV' in dst:
            put_csv_output(dst)
        if 'postgres' in dst:
            put_postgres_output(dst)

    finally:
        for f in glob.glob(INPUT_PATH + '/*csv'):
            os.remove(f)
        if os.path.isdir(DB_PATH):
            shutil.rmtree(DB_PATH)
        if os.path.isfile(OUTPUT_PATH):
            os.remove(OUTPUT_PATH)


if __name__ == '__main__':
    src = 'src_test.json'
    dst = 'dst_test.json'
    execute_imputedb(src, dst, 'select white_blood_cell_ct from labs;', 0)

