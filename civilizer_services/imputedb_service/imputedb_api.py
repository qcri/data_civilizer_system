from __future__ import print_function

import json

SELF_PATH = os.path.realpath(__file__)
STORAGE_PATH = SELF_PATH + '/../../storage/imputedb/'
IMPUTEDB_PATH = SELF_PATH + '/imputedb/imputedb'

def get_csv_paths(src):
    csv_paths = []
    csv_dir = src['CSV']['dir'] + '/'
    csv_tables = src['CSV']['table'].split(';')

    assert os.path.isdir(csv_dir)
    for fn in os.listdir(csv_dir):
        csv_paths = csv_dir + fn
    return csv_paths


def get_postgres_paths(src, tmp_dir):
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
        csv_fn = '{}/{}.csv'.format(tmp_dir, table)
        with open(csv_fn, 'w') as f:
            p = subprocess.Popen(table_cmd, env=env, stdout=f)
            p.wait()
            if p.returncode < 0:
                raise RuntimeError('psql failed with return code {}'\
                                   .format(p.returncode))
        csv_paths += [csv_fn]
    return csv_paths


def put_csv_output(dst, tmp_out):
    out_dir = dst['CSV']['dir']
    shutil.copy(tmp_out, out_dir)


def put_postgres_output(dst, tmp_out):
    raise RuntimeError('Not implemented.')


def execute_imputedb(src_json, dst_json, query, alpha):
    src = json.loads(src_json)
    dst = json.loads(dst_json)

    try:
        tmp_dir = tempfile.mkdtemp(dir=STORAGE_PATH) + '/'
        tmp_db = tempfile.mkdtemp(dir=STORAGE_PATH, suffix='.db')
        tmp_out = tempfile.mkstemp(dir=STORAGE_PATH, suffix='.csv')

        csv_paths = []
        if 'CSV' in src:
            cvs_paths += get_csv_paths(src)
        if 'postgres' in src:
            csv_paths += get_postgres_paths(src, tmp_dir)

        load_cmd = [IMPUTEDB_PATH, 'load', '--db', tmp_db] + csv_paths
        subprocess.check_call(load_cmd)

        query_cmd = [IMPUTEDB_PATH, 'query', '--db', tmp_db, '--csv', '-c', query]
        with open(tmp_out, 'w') as f:
            subprocess.check_call(query_cmd, stdout=f)

        if 'CSV' in dst:
            put_csv_output(dst, tmp_out)
        if 'postgres' in dst:
            put_postgres_output(dst, tmp_out)

    finally:
        if os.path.isdir(tmp_dir):
            shutil.rmtree(tmp_dir)
        if os.path.isdir(tmp_db):
            shutil.rmtree(tmp_db)
        if os.path.isfile(tmp_out):
            os.remove(tmp_out)


if __name__ == '__main__':
    src = 'src_test.json'
    dst = 'dst_test.json'
    execute_imputedb(src, dst, '', 0)
