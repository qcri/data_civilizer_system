FROM python:3.5
MAINTAINER Essam Mansour <essam.mansour@gmail.com>
WORKDIR /app

COPY ./requirements.txt /app/requirements.txt
RUN pip install -r requirements.txt

RUN apt-get update -qq && \
    apt-get install -y \
    mono-complete wget build-essential unzip sudo && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


#RUN add-apt-repository ppa:webupd8team/java && \
#    apt-get update && \
#    apt-get install oracle-java8-installer

COPY ./rest /app/rest
RUN /app/rest/services/build.sh
RUN /app/rest/services/deeper_service/DeepER-Lite/install.sh

EXPOSE 8089
CMD python ./rest/civilizer.py
