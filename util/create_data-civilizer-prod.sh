#!/bin/bash

#2345678901234567890123456789012345678901234567890123456789012345678901234567890

if [ $# -ne 2 ]
then
    echo "Usage is: $0 [source-image] [dest-image]"
    echo
    echo "  'source-image' must be an existing docker image"
    echo "  'dest-image' will be a new image created via docker import"
    exit 1
fi

tmp_container=tmp-$(tr -cd '0-9a-z' < /dev/urandom | head -c 28)

docker run --name ${tmp_container} --entrypoint /bin/bash -i $1 <<-'__EOF__'
	# Purge key packages that were manually installed, and any packages
	# that were automatically installed and are no longer necessary.
        #
        # Note: The inclusion of the 'dpgk-query' command in the list of
        #       packages, combined with the 'sort' and 'uniq' filters used
        #       to process the list, results in the list being filtered to
        #       only include packages that are currently installed, thus
        #       avoiding any error caused by packages that are not installed.
        #
	apt-get purge --auto-remove -y $(cat << __PKGLIST__ | tr -d ' ' | sort | uniq -d
	    $(dpkg-query -W -f '${package}\n')
	    ant
	    cuda-command-line-tools-8-0
	    cuda-cublas-dev-8-0
	    cuda-cudart-dev-8-0
	    cuda-cufft-dev-8-0
	    cuda-curand-dev-8-0
	    cuda-cusolver-dev-8-0
	    cuda-cusparse-dev-8-0
	    cuda-driver-dev-8-0
	    cuda-npp-dev-8-0
	    cuda-nvgraph-dev-8-0
	    cuda-nvml-dev-8-0
	    cuda-nvrtc-dev-8-0
	    g++-4.8
	    gcc-4.8
	    gfortran
	    gnuplot
	    gnuplot-x11
	    imagemagick
	    ipython
	    lib32ncurses5-dev
	    libcudnn5-dev
	    libfreetype6-dev
	    libgraphicsmagick1-dev
	    libjpeg-dev
	    libncurses5-dev
	    libpng-dev
	    libqt4-dev
	    libreadline-dev
	    libsox-dev
	    libsox-fmt-all
	    libzmq3-dev
	    python3-dev
	    python3-pip
	    sox
	    swig
	    vim
	__PKGLIST__
	)
	# Restore the python link if it was removed by the packages above
	[ -f /usr/bin/python ] || ln -s python3 /usr/bin/python
	exit
__EOF__

Hostname="$(     docker inspect -f "{{ .Config.Hostname }}"          $1)"
Domainname="$(   docker inspect -f "{{ .Config.Domainname }}"        $1)"
User="$(         docker inspect -f "{{ .Config.User }}"              $1)"
ExposedPorts="$( docker inspect -f "{{ json .Config.ExposedPorts }}" $1 | sed -r -e 's/^null$//' -e 's/,/ /g' -e 's/[{"}:]//g')"
Env="$(          docker inspect -f "{{ json .Config.Env }}"          $1 | sed -r -e 's/","/ /g' -e 's/^\[|\]$//g')"
Cmd="$(          docker inspect -f "{{ json .Config.Cmd }}"          $1 | sed -r -e 's/^null$//')"
Volumes="$(      docker inspect -f "{{ json .Config.Volumes }}"      $1 | sed -r -e 's/^null$//' -e 's/","/ /g' -e 's/^\[|\]$//g')"
WorkingDir="$(   docker inspect -f "{{ json .Config.WorkingDir }}"   $1)"
Entrypoint="$(   docker inspect -f "{{ json .Config.Entrypoint }}"   $1 | sed -r -e 's/^null$//')"
Labels="$(       docker inspect -f "{{ json .Config.Labels }}"       $1 | sed -r -e 's/("[^"]*"):("[^"]*"),?/\1=\2 /g' -e 's/^\{|\}$//g')"

changes=( ' ' )
# -n "$Hostname" ]     && changes+=" --change 'HOSTNAME ${Hostname}'"
# -n "$Domainname" ]   && changes+=" --change 'DOMAINNAME ${Domainname}'"
[ -n "$User" ]         && changes+=" --change 'USER ${User}'"
[ -n "$ExposedPorts" ] && changes+=" --change 'EXPOSE ${ExposedPorts}'"
[ -n "$Env" ]          && changes+=" --change 'ENV ${Env}'"
[ -n "$Cmd" ]          && changes+=" --change 'CMD ${Cmd}'"
[ -n "$Volumes" ]      && changes+=" --change 'VOLUME ${Volumes}'"
[ -n "$WorkingDir" ]   && changes+=" --change 'WORKDIR ${WorkingDir}'"
[ -n "$Entrypoint" ]   && changes+=" --change 'ENTRYPOINT ${Entrypoint}'"
[ -n "$Labels" ]       && changes+=" --change 'LABEL ${Labels}'"

docker export ${tmp_container} | bash -c "docker import ${changes[@]} - $2"

docker rm ${tmp_container}
