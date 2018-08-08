#!/bin/bash

#2345678901234567890123456789012345678901234567890123456789012345678901234567890

cfg_profile=${1:-init}

config() {
    case "$cfg_profile" in
        init)
            cfg=(
                profiler:        aurum/ddprofiler:0.1
                nbc:             aurum/networkbuildercoordinator:0.1
                apis:            data-civilizer-api:0.1
                grecord_service: grecord_service:0.1a
                studio:          data-civilizer-studio:0.1
            )
            ;;
        dev)
            cfg=(
                profiler:        aurum/ddprofiler:0.4
                nbc:             aurum/networkbuildercoordinator:0.3
                apis:            data-civilizer-api:0.9
                grecord_service: grecord_service:0.7
                studio:          data-civilizer-studio:0.4
            )
            ;;
        prod)
            cfg=(
                profiler:        aurum/ddprofiler-prod:0.4
                nbc:             aurum/networkbuildercoordinator-prod:0.3
                apis:            data-civilizer-api-prod:0.9
                grecord_service: grecord_service-prod:0.7
                studio:          data-civilizer-studio-prod:0.4
            )
            ;;
        special)
            cfg=(
                profiler:        aurum/ddprofiler:0.1
                nbc:             aurum/networkbuildercoordinator:0.1
                apis:            data-civilizer-api:0.1
                grecord_service: grecord_service:0.8
                studio:          data-civilizer-studio:0.1
            )
            ;;
    esac
    yaml="$(cat ./docker-compose.yml)"
    i=0
    while [ $i -lt ${#cfg} ]
    do
      yaml=$(sed -r "/^ *${cfg[i]}/,/^ *image:/ s/^( *image:).*$/\1 ${cfg[i+1]//\//\\/}/" <<< "$yaml")
      ((i+=2))
    done
    echo "$yaml"
}

report() {
    # To do:
    # --Problems here if the curl call failed and no output files were generated...
    # --Changing Python versions changed some outputs, but may still be valid -- use regex to accept multiple check values?
    [ ${#2} -lt 32 ] && chk=$3 || chk=$(md5sum $(eval ls $3) | tee /dev/tty | md5sum | tee /dev/tty | cut -d' ' -f1)
    [ "$2" = "$chk" ] && result="SUCCESS" || result="FAILURE"
    echo
    echo "********************************************************"
    echo "*                                                      *"
    printf "*   Test: %-24s   Status: %s   *\n" $1 $result
    echo "*                                                      *"
    echo "********************************************************"
    echo
    echo
}

clean() {
    cd ~/data_civilizer/source_code/aurum-datadiscovery/

    # aurum/ddprofiler
    rm -rf ./data/elasticsearch/nodes/*

    # aurum/networkcoordinatorbuilder
    rm -f ./data_civilizer/source_code/aurum-datadiscovery/data/models/*

    cd ~/data_civilizer/source_code/data_civilizer_system/

    # data-civilizer-api:Fahes
    rm -f ./app_storage/data_sets/MIT_DemoResults/DMV_*.csv

    # data-civilizer-api:ImputeDB
    rm -f ./app_storage/data_sets/MIT_DemoResults/out.csv

    # data-civilizer-api:PKDuck
    rm -f ./app_storage/data_sets/MIT_DemoResults/{*_pkduck,updated_*}.csv

    # data-civilizer-api:DeepER
    rm -f ./app_storage/data_sets/deeper/output/pred_pairs_2500_0.csv

    # grecord_service
    rm -f ./app_storage/data_sets/deeper/output/GRRows_*.csv
}

#
# Test: aurum
#

aurum-test() {
    cd ~/data_civilizer/source_code/aurum-datadiscovery
    eval rm -rf $2
    docker-compose -f <(config) --project-directory . up --exit-code-from $1 $1
    report aurum/$1 0 $?
}

#
# Test: aurum/ddprofiler
#
# Note: Aurum/ddprofiler reads its configuration settings (including the input
#       source directory) from a yaml file specified by the '--sources' option
#       passed to the entrypoint of the container from the docker-compose.yml
#       file.  The output of the processed sources is written into the
#       elasticsearch database, which makes comparison of the results from
#       consecutive runs difficult.  This test relies solely on the exit code
#       from the profiler service to validate success.
#
test-ddprofiler() {
    aurum-test \
        profiler \
        './data/elasticsearch/nodes/*'
}

#
# Test: aurum/networkbuildercoordinator
#
# Note: The aurum/nbc reads its input from the elasticsearch database created
#       by the aurum/ddprofiler.  It generates an output model stored in
#       Python pickle format.  Unfortunately, the model appears to use
#       randomly generated identifiers resulting in non-deterministic output.
#       As with the aurum/ddprofiler test above, this one relies soley on the
#       nbc serice exit code to determine success.
#
test-nbc() {
    aurum-test \
        nbc \
        './data_civilizer/source_code/aurum-datadiscovery/data/models/*'
}

#
# Test: data-civilizer-api
#

#
# Expected input test data:
#
# md5sum $(ls ./app_storage/data_sets/MIT_Demo/*.csv ./app_storage/data_sets/deeper/Amazon-GoogleProducts/*_full.csv)
# 2a2d60c4b9a6f15d5d867367bcc0c85e  ./app_storage/data_sets/MIT_Demo/Drupal_employee_directory.csv
# fe37bc0087e512c7e4e9300bdb5c4ca6  ./app_storage/data_sets/MIT_Demo/Employee_directory.csv
# 7946786665ad91ec1546f4de018fc805  ./app_storage/data_sets/MIT_Demo/Mit_student_directory.csv
# 39306ff1bdb0be410f98635d6b06c6d8  ./app_storage/data_sets/MIT_Demo/Se_person.csv
# b12a98e9aad9a43fd972a056189eb0cf  ./app_storage/data_sets/MIT_Demo/Sis_department.csv
# 077e30e0a046982e28806ca7e2cfca67  ./app_storage/data_sets/MIT_Demo/Warehouse_users.csv
# 00a44b0f13ffd22a535dc447c7a1bf8b  ./app_storage/data_sets/MIT_Demo_IM/Sis_department.csv
# 7fb5ec0599f10f0bfc08402ec143ab59  ./app_storage/data_sets/deeper/Amazon-GoogleProducts/Amazon_full.csv
# e65ada1c9f3d337a7343e36c3751095e  ./app_storage/data_sets/deeper/Amazon-GoogleProducts/GoogleProducts_full.csv
#
# md5sum $(ls ./app_storage/data_sets/MIT_Demo/*.csv ./app_storage/data_sets/deeper/Amazon-GoogleProducts/*_full.csv) | md5sum
# 32d2703371c28c726ce709fd40589ec5  -
#
# -- Validate input files before test?
#

data-civilizer-api-test() {
    check="${@: -2: 1}"
    output="${!#}"
    eval rm -f $output
    set -- "${@: 1: $#-2}"
    declare -A params=([param0]='' [param1]='')
    i=2
    while [ -n "${!i}" ]
    do
        params["param$i"]="${!i}"
        ((i++))
    done
    p="$(typeset -p params | sed -r -e "s/^.*\(| \)'$//g" -e 's/\[|\]/"/g' -e 's/=/:/g' -e 's/" "/","/g')"
    data="{\"operators\":[{\"java_class\":\"civilizer.basic.operators.$1\",\"parameters\":{$p}}]}"
    port=8089
    [ "$1" = "EntityConsolidation" ] && port=8889
    echo "curl -v -s -S -H 'Content-Type: application/json' -d '$data' http://localhost:$port/rheem/plan_executions"
    status=$(
        curl -v -s -S -H 'Content-Type: application/json' -d "$data" http://localhost:$port/rheem/plan_executions 2>&1 |
        tee /dev/tty |
        grep '^< HTTP/1.[0-9] [1-5][0-9][0-9] ' |
        awk '{print $3}'
    )
    [ ${#check} -lt 32 ] && output=$status
    report $1 $check "$output"
}

#
# Subtest: Fahes
#
# Expected test output:
# md5sum $(ls ./app_storage/data_sets/MIT_DemoResults/DMV_*.csv)
# 9a2f6337f60951e1982d635d3d9afde3  ./app_storage/data_sets/MIT_DemoResults/DMV_Drupal_employee_directory.csv
# 6ced11475b3b311a8dd9f2556199683c  ./app_storage/data_sets/MIT_DemoResults/DMV_Employee_directory.csv
# 85e492000c5de732009c8d412c50d5bf  ./app_storage/data_sets/MIT_DemoResults/DMV_Mit_student_directory.csv
# d41d8cd98f00b204e9800998ecf8427e  ./app_storage/data_sets/MIT_DemoResults/DMV_Se_person.csv
# 472a3d899e2f485c4fc1af4458fb1a40  ./app_storage/data_sets/MIT_DemoResults/DMV_Sis_department.csv
# 0bc726e0627f6c05b9aec1d2d52253cc  ./app_storage/data_sets/MIT_DemoResults/DMV_Warehouse_users.csv
#
# md5sum $(ls ./app_storage/data_sets/MIT_DemoResults/DMV_*.csv) | md5sum
# 49d2fac7d073fa39c63222af4ea98435  -
#
test-fahes() {
    data-civilizer-api-test                     \
        DataCleaning-Fahes                      \
        /app/storage/data_sets/MIT_Demo/        \
        /app/storage/data_sets/MIT_DemoResults/ \
                                                \
        49d2fac7d073fa39c63222af4ea98435        \
        './app_storage/data_sets/MIT_DemoResults/DMV_*.csv'
}

#
# Subtest: ImputeDB
#
# Expected test output:
# md5sum $(ls ./app_storage/data_sets/MIT_DemoResults/out.csv)
# ac0ac982859d3cf503eb331feb6f068b  ./app_storage/data_sets/MIT_DemoResults/out.csv
#
# md5sum $(ls ./app_storage/data_sets/MIT_DemoResults/out.csv) | md5sum
# af92679c8988c95cc2f52d3afda8b124  -
#
test-imputedb() {
    data-civilizer-api-test                            \
        DataCleaning-Imputedb                          \
        /app/storage/data_sets/MIT_Demo_IM/            \
        /app/storage/data_sets/MIT_DemoResults/        \
        Sis_department                                 \
        "select Dept_Budget_Code from Sis_department;" \
        0                                              \
                                                       \
        af92679c8988c95cc2f52d3afda8b124               \
        './app_storage/data_sets/MIT_DemoResults/out.csv'
}

#
# Subtest: PKDuck
#
# Expected test output:
# md5sum $(ls ./app_storage/data_sets/MIT_DemoResults/{*_pkduck,updated_*}.csv)
# 58ec991b88f6648ec4f1b2912d9645ad  ./app_storage/data_sets/MIT_DemoResults/auxiliary_pkduck.csv
# 2dd8a4e826a27b6c249ae81354ed65dc  ./app_storage/data_sets/MIT_DemoResults/simstring_pkduck.csv
# daae37e14ccbb2bb94471a77c3beb556  ./app_storage/data_sets/MIT_DemoResults/updated_Drupal_employee_directory.csv
# 22946ac1269aca2461a7105817396b37  ./app_storage/data_sets/MIT_DemoResults/updated_Employee_directory.csv
# 40b64a6133402ec330ff2f492020534b  ./app_storage/data_sets/MIT_DemoResults/updated_Mit_student_directory.csv
# 10fac4dde70ca73005433860aa1819bd  ./app_storage/data_sets/MIT_DemoResults/updated_Se_person.csv
# ef5a01841fd8f94895000a8efae9be6a  ./app_storage/data_sets/MIT_DemoResults/updated_Sis_department.csv
# eb236c01ff308005de1a90ef0c93bdf9  ./app_storage/data_sets/MIT_DemoResults/updated_Warehouse_users.csv
#
# md5sum $(ls ./app_storage/data_sets/MIT_DemoResults/{*_pkduck,updated_*}.csv) | md5sum
# cc7787c70b52167a0c89cf4269609d59  -
#
test-pkduck() {
    data-civilizer-api-test                     \
        DataCleaning-PKDuck                     \
        /app/storage/data_sets/MIT_Demo/        \
        /app/storage/data_sets/MIT_DemoResults/ \
        8#11#12                                 \
        0.8                                     \
                                                \
        cc7787c70b52167a0c89cf4269609d59        \
        './app_storage/data_sets/MIT_DemoResults/{*_pkduck,updated_*}.csv'
}

#2345678901234567890123456789012345678901234567890123456789012345678901234567890
#
# Subtest: DeepER
#
# Note: DeepER apparently uses random sampling to select records from the input
#       set, thus generating non-deterministic output which prevents simple
#       verification checking.  Also, the API call consistently terminates with
#       an error code that results in an HTTP status code of 500, "INTERNAL
#       SERVER ERROR".
#
test-deeper() {
    data-civilizer-api-test                                 \
        EntityMatching-DeepER                               \
        /app/storage/data_sets/deeper/Amazon-GoogleProducts \
        /app/storage/data_sets/deeper/output                \
        Amazon_full.csv GoogleProducts_full.csv             \
        "select Dept_Budget_Code from Sis_department;"      \
        2500                                                \
                                                            \
        500                                                   \
        './app_storage/data_sets/deeper/output/pred_pairs_2500_0.csv'
}

#
# Test: grecord_service
#
# Expected test output:
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_address.csv)
# ad502c82e9535a517fc54a04f67c5e20  ./app_storage/data_sets/deeper/output/GRRows_address.csv
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_address.csv) | md5sum
# c8bb52ba374233c85d12b3e148e67b58  -
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_title.csv)
# 6c6507a77bfc35d4ada5bc2850f578ef  ./app_storage/data_sets/deeper/output/GRRows_title.csv
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_title.csv) | md5sum
# ce0f450191deec5cc9ff16ad054638a6  -
#
# Expected test output (v0.7):
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_*.csv)
# f304ef818e818b91fe0b6158f4f8d51b  ./app_storage/data_sets/deeper/output/GRRows_address.csv
# 1d54a107e1e79b3ce8aeef558277dab4  ./app_storage/data_sets/deeper/output/GRRows_title.csv
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_*.csv) | md5sum
# 1128be64b09627ad30cde22e2673255c  -
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_address.csv)
# f304ef818e818b91fe0b6158f4f8d51b  ./app_storage/data_sets/deeper/output/GRRows_address.csv
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_address.csv) | md5sum
# b6159d8bac0ca86c6f264e56739fd8ba  -
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_title.csv)
# 1d54a107e1e79b3ce8aeef558277dab4  ./app_storage/data_sets/deeper/output/GRRows_title.csv
#
# md5sum $(ls ./app_storage/data_sets/deeper/output/GRRows_title.csv) | md5sum
# 24696218df127650021a247f47c6b357  -
#
test-grecord() {
    data-civilizer-api-test                                     \
        EntityConsolidation                                     \
        /app/storage/data_sets/gr/address/                      \
        /app/storage/data_sets/deeper/output/GRRows_address.csv \
        0000000000000000314                                     \
                                                                \
        c8bb52ba374233c85d12b3e148e67b58                        \
        './app_storage/data_sets/deeper/output/GRRows_address.csv'
    data-civilizer-api-test                                     \
        EntityConsolidation                                     \
        /app/storage/data_sets/gr/title/                        \
        /app/storage/data_sets/deeper/output/GRRows_title.csv   \
        0514                                                    \
                                                                \
        ce0f450191deec5cc9ff16ad054638a6                        \
        './app_storage/data_sets/deeper/output/GRRows_title.csv'
}

# Test: data-civilizer-studio




cd ~/data_civilizer/source_code/aurum-datadiscovery
docker-compose up -d elasticsearch
while ! nc -z 127.0.0.1 9200
do
    sleep 1
done
test-ddprofiler
test-nbc
docker-compose down

cd ~/data_civilizer/source_code/data_civilizer_system
docker-compose -f <(config) --project-directory . up -d studio
test-fahes
test-imputedb
test-pkduck
test-deeper
test-grecord
docker-compose down
