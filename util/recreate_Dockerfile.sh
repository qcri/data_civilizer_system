#!/bin/bash

# https://docs.docker.com/engine/reference/builder/

if [ $# -ne 1 ]
then
    echo "Usage: $0 [image]"
    exit 1
fi

from=$(docker inspect -f "{{ json .RepoTags }}" $(docker history $1 | sed 's/ .*$//;/<missing>/!h;g;$!d') | sed 's/^[^"]*"\|".*$//g')
echo "FROM $from"

docker history --no-trunc $1 |
sed -r 's/^([^ ]+) +(CREATED|.+ ago) +(CREATED BY|\/bin.+[^ ]) +(SIZE|[0-9.]+ [kMG]?B) +(.*)?$/\1\t\2\t\3\t\4\t\5/' |
tail -n +2 |
tac |
sed -r '1,/^[^<]/ s/^([^\t]*\t[^\t]*\t)/\1#/' |
cut -f3 |
sed -r 's/^(#?)\/bin\/sh -c/\1RUN/' |
sed -r 's/^(#?)RUN #\(nop\) (ADD|COPY)/#\2/' |
sed -r 's/^(#?)RUN #\(nop\)  ?(CMD|ENTRYPOINT|ENV|EXPOSE|LABEL|MAINTAINER|USER|WORKDIR)/\1\2/' |
sed -r '/^#?(CMD|USER)/ s/\[(.*)\]/\1/g' |
sed '/^#?ENTRYPOINT/ s/" "/", "/g' |
sed '/^#?LABEL/ s/=\(.*\)$/="\1"/' |
sed '/^#RUN/ s/     / \\\n#    /g' |
sed '/^#RUN/ s/  &&/ \\\n# \&\&/g' |
sed '/^RUN/ s/     / \\\n    /g' |
sed '/^RUN/ s/  &&/ \\\n \&\&/g' |
sed 's/   *\\$/ \\/'
