FROM ubuntu:18.04
MAINTAINER Essam Mansour <essam.mansour@gmail.com>

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        nodejs \
        npm \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY package.json /app/package.json
RUN npm install -g gulp-cli && \
    npm install
COPY . /app
EXPOSE 5000
CMD npm start
