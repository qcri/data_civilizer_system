FROM node:latest
MAINTAINER Essam Mansour <essam.mansour@gmail.com>
WORKDIR /app
COPY package.json /app/package.json
RUN npm install -g gulp-cli && \
	npm install
COPY . /app
EXPOSE 5000
CMD npm start
