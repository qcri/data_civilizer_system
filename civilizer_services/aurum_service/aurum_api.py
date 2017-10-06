from main import init_system
from api.apiutils import Relation
import loc_config

global api
api = None

def init():
    global api
    api, reporting = init_system(loc_config.ekg_path)
    return api, reporting


def run_query1_keyword(keyword):
    global api
    res = api.keyword_search(keyword)
    res.set_table_mode()
    tables = [el for el in res]
    tables_string = ";".join(tables)
    return tables


def run_query2_schema(schema_name):
    #tokens = schema_name.split(",")
    #for t in tokens:
    global api
    res = api.schema_name_search(t)
    res.set_table_mode()
    tables = [el for el in res]
    tables_string = ";".join(tables)
    return tables


def configuration(config):
           return 1


def execute(source, destination):
           return 1
