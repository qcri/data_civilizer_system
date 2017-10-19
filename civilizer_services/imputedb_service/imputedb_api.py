from __future__ import print_function

import glob
import json
import os
import shutil
import subprocess
import tempfile

SELF_DIR_PATH = os.path.dirname(os.path.realpath(__file__))
STORAGE_PATH = SELF_DIR_PATH + '/../../storage/imputedb/'
IMPUTEDB_PATH = SELF_DIR_PATH + '/imputedb/imputedb'
DB_PATH = STORAGE_PATH + '/tmp.db'
INPUT_PATH = STORAGE_PATH + '/inputs'
OUTPUT_PATH = STORAGE_PATH + '/out.csv'

def get_csv_paths(src):
    csv_paths = []
    csv_dir = src['CSV']['dir'] + '/'
    csv_tables = src['CSV']['table']

    assert csv_tables != '', 'Must specify some tables'

    assert os.path.isdir(csv_dir)
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


def execute_imputedb(src_json, dst_json, query, alpha):
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
