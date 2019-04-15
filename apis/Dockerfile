FROM llsc/cuda-torch:0.6
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
RUN python3 -m pip install funcy

# fix the path for python
# RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.5 10
RUN ln -sf python3 /usr/bin/python
RUN python --version
RUN pip --version

# Add the JDK 8 and accept licenses (mandatory)
RUN add-apt-repository ppa:webupd8team/java && \
    echo debconf shared/accepted-oracle-license-v1-1 select true | debconf-set-selections && \
    echo debconf shared/accepted-oracle-license-v1-1 seen true | debconf-set-selections

# Install Java 8
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        oracle-java8-installer \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install ant for ImputeDB, should be installed after Java8
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ant \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# install needed libs for python
COPY ./requirements.txt /app/requirements.txt
RUN pip3 install -r requirements.txt

# Build Torch -- No need
# COPY ./rest/services/deeper_service/DeepER-Lite /app/DeepER-Lite
# RUN /app/DeepER-Lite/install.sh

# install required lib for DeepER
#demo-m# RUN luarocks install csvigo
#demo-m# RUN luarocks install dp

COPY ./rest /app/rest

# -- Modify build.sh in place to not install GloVe
# -- Create symlink to point to external GloVe.t7 file
#demo-m# RUN sed -i '/GloVe/,/^)$/ s/^[^#]/#&/' /app/rest/services/build.sh && \
#demo-m#     /app/rest/services/build.sh && \
#demo-m#     find /app -name '*.o' -exec rm -f {} \; && \
#demo-m#     ln -s /app/storage/glove/GloVe.t7 /app/rest/services/deeper_service/DeepER-Lite/glove/GloVe.t7

#demo-m# COPY ./fastText /root/fastText

#demo-m# RUN cd /root/fastText && \
#demo-m#     python3 setup.py install

EXPOSE 8089
#CMD python ./rest/civilizer.py
CMD ./rest/civilizer.sh
