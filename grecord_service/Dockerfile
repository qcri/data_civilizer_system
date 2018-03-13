# FROM python:3.5
# MAINTAINER Essam Mansour <essam.mansour@gmail.com>
FROM ubuntu:16.04
MAINTAINER Essam Mansour <essam.mansour@gmail.com>

RUN apt-get update -qq && apt-get upgrade -y && \
    apt-get install -y \
    software-properties-common build-essential python3.5 python3.5-dev python3-pip python3.5-venv \
    swig vim unzip sudo mono-complete git wget && \
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

# RUN apt-get -y install ipython ipython-notebook

RUN echo "create app dir ..."

WORKDIR /app

RUN echo "install req ..."
COPY ./requirements.txt /app/requirements.txt
RUN pip install -r requirements.txt



COPY ./gr /app
RUN /app/build.sh

RUN mkdir ~/.jupyter
RUN echo "c.NotebookApp.token = u''" >> ~/.jupyter/jupyter_notebook_config.py


EXPOSE 8888
CMD jupyter notebook --allow-root --NotebookApp.token= --ip=0.0.0.0 --port=8888 /app/code/civilizer_gr.ipynb
