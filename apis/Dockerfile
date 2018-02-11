# FROM python:3.5
# FROM ubuntu:16.04
FROM nightseas/cuda-torch
MAINTAINER Essam Mansour <essam.mansour@gmail.com>

RUN apt-get update -qq && apt-get upgrade -y && \
    apt-get install -y \
    software-properties-common build-essential python3.5 python3.5-dev python3-pip python3.5-venv \
    vim unzip sudo mono-complete git wget && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# update pip
RUN python3.5 -m pip install pip --upgrade
RUN python3.5 -m pip install wheel

# fix the path for python
# RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.5 10
RUN ln -sf python3.5 /usr/bin/python
RUN python --version
RUN pip --version

# Add the JDK 8 and accept licenses (mandatory)
RUN add-apt-repository ppa:webupd8team/java && \
    echo debconf shared/accepted-oracle-license-v1-1 select true | debconf-set-selections && \
    echo debconf shared/accepted-oracle-license-v1-1 seen true | debconf-set-selections

# Install Java 8
RUN apt-get update && \
    apt-get --yes --no-install-recommends install oracle-java8-installer

WORKDIR /app

# install needed libs for python
COPY ./requirements.txt /app/requirements.txt
RUN pip install -r requirements.txt


# Build Torch -- No need
# COPY ./rest/services/deeper_service/DeepER-Lite /app/DeepER-Lite
# RUN /app/DeepER-Lite/install.sh
RUN luarocks install csvigo
RUN luarocks install dp


RUN apt-get install -y ant
COPY ./rest /app/rest
RUN /app/rest/services/build.sh


EXPOSE 8089
CMD python ./rest/civilizer.py