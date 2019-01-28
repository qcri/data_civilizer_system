FROM ubuntu:18.04
MAINTAINER Essam Mansour <essam.mansour@gmail.com>

RUN DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        python3 \
        python3-dev \
        python3-pip \
        python3-setuptools \
        python3-tk \
        vim \
        unzip \
        wget \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# update pip
RUN python3 -m pip install pip --upgrade
RUN python3 -m pip install wheel

# fix the path for python
RUN ln -sf python3 /usr/bin/python
RUN python --version
RUN pip --version

WORKDIR /app

# install needed libs for python
COPY ./requirements.txt /app/requirements.txt
RUN pip3 install -r requirements.txt

COPY ./rest /app/rest

EXPOSE 8090
CMD python ./rest/civilizer.py
