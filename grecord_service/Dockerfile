FROM ubuntu:18.04
MAINTAINER Essam Mansour <essam.mansour@gmail.com>

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
        build-essential \
        python3 \
        python3-dev \
        python3-pip \
        python3-setuptools \
        swig \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# update pip
RUN python3 -m pip install pip --upgrade
RUN python3 -m pip install wheel

# fix the path for python
# RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.5 10
RUN ln -sf python3 /usr/bin/python
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

EXPOSE 8888 8889
CMD (jupyter notebook --allow-root --NotebookApp.token= --ip=0.0.0.0 --port=8888 /app/code/civilizer_gr.ipynb &); python ./code/grecord_service.py
