#!/usr/bin/python
from datetime import datetime
import subprocess
import os

#CODE_DIR = '/data/dongdeng/vldb18-goldenrecord/code'
CODE_DIR = '/Users/dongdeng/Dropbox/project/goldenrecord/code/enum_rule'

#global_freq="-DGLOBAL_FREQUENCY_THRESHOLD 10 "
#local_freq="-DLOCAL_FREQUENCY_THRESHOLD 0.2 "
#path_length="-DMAX_PATH_LENGTH 4 "
#rule_num="-DMAX_NUMBER_OF_RULES 10000 "

#4.3
rev_map="-DREVERSE_MAPPING_DIR_ENABLE "
rand_map="-DRANDOM_MAPPING_DIR_ENABLE "
long_map="-DLONG_FIRST_MAPPING_ENABLE "

#4
no_agg="-DNO_AGGREGATION_ENABLE "
both_agg="-DBOTH_AGGREGATION_ENABLE "
trans_agg="-DTRANSFORM_AGGREGATION_ENABLE "
struct_agg="-DSTRUCTURE_AGGREGATION_ENABLE "

#5.1
local_thrsh="-DLOCAL_THRESHOLD_ENABLE "
global_thrsh="-DGLOBAL_THRESHOLD_ENABLE "
unique_thrsh="-DUNIQUE_THRESHOLD_ENABLE "

#5.2
static="-DSTATIC_ORDERING_ENABLE "
constant="-DSINGLE_CONSTANT_TERM_ENABLE "
prefix="-DPREFIX_SUFFIX_TERM_ENABLE "

#6
rule_update="-DRULE_UPDATE_ENABLE "
rule_reapply="-DREAPPLY_RULE_ENABLE "

default_map="-DBEST_MAPPING_ENABLE "
default_agg=both_agg
default_thrsh = local_thrsh + global_thrsh + unique_thrsh
default_adapt = static + constant + prefix
default_apply="-DBEST_APPLY_ENABLE "

no_threshold = "-DNO_THRESHOLD "
output = "-DOUTPUT_INFO "

make_prefix = "make -f makefile-so -j all OPTION=\""
make_suffix = "\""

AGGS = [both_agg, struct_agg, trans_agg, no_agg]
MAPPINGS = [default_map, rev_map, rand_map, rand_map + long_map]
THRESHOLDS = [local_thrsh, default_thrsh, global_thrsh + unique_thrsh, no_threshold]

#THRESHOLDS = [local_thrsh, global_thrsh, unique_thrsh, local_thrsh + global_thrsh, unique_thrsh + global_thrsh, local_thrsh + unique_thrsh, local_thrsh + global_thrsh + unique_thrsh]
ADAPTS = [constant, constant + static, constant + prefix, constant + static + prefix]
APPLYS = [default_apply, rule_update]
DATASET = ["../data/book/ bookname 1", "../data/book/ author 2", "../data/citation/ proceedings 3", "../data/tamr/ address 4"]
RULESET = ["../data/book/ bookname 3", "../data/book/ author 4", "../data/citation/ proceedings 2", "../data/tamr/ address 1"]

if __name__ == '__main__':
  make_infix = default_map + default_agg + default_thrsh + default_adapt + default_apply + rule_update #+ output
  make_cmd = make_prefix + make_infix + make_suffix
  print(make_cmd)
  os.system("rm *.o")
  os.system(make_cmd)


  #make_infix = default_map + default_agg + default_thrsh + default_adapt + default_apply


