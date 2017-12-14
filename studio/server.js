var express = require('express');
var mongoose = require('mongoose');
var bodyParser = require('body-parser');
var path = require('path');
var fs = require('fs');
var WebSocket = require("./helpers/websocket");

// Mongodb connection
mongoose.Promise = global.Promise;
db = process.env.MONGODB_URL;
connection = mongoose.connect(db);

// init

app = express();

app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

var socketServerPort = process.env.SOCKET_SERVER_PORT;
websocket = WebSocket(socketServerPort);

console.log("websocket is running on port: ", socketServerPort);

//config
app.use("/bower_components", express.static(__dirname + '/bower_components'));
app.use("/js", express.static(__dirname + '/public/js'));
app.use("/css", express.static(__dirname + '/public/css'));
app.use("/libs", express.static(__dirname + '/public/libs'));
app.use("/views", express.static(__dirname + '/public/views'));
app.use("/images", express.static(__dirname + '/public/images'));


// Routes
app.use('/api', require('./routes/api'));

// Index route
app.get('/', function(req, res){
    res.sendFile(path.join(__dirname, './public', 'index.html'));
});

//load port number
var port = process.env.WEB_SERVER_PORT;

//start server
app.listen(port);

console.log("API is running on port: ", port);
