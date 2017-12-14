FROM python:3.5
MAINTAINER Essam Mansour <essam.mansour@gmail.com>
WORKDIR /app
COPY ./requirements.txt /app/requirements.txt
RUN pip install -r requirements.txt

COPY ./rest /app/rest
RUN /app/rest/services/build.sh

EXPOSE 8089
CMD python ./rest/civilizer.py
