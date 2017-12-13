
var webSocketServer = require('websocket').server;
var http = require('http');
var httpProxy = require('http-proxy');
var events = require('events');

var server = http.createServer(function(request, response) {
});

server.listen(5002, function() {
    console.log((new Date()) + " Server is listening on port 5002");
});

var wsServer = new webSocketServer({
    httpServer: server
});



wsServer.on('request', function(request) {
  var eventEmitter = new events.EventEmitter();
  var connection = request.accept(null, request.origin);

  eventEmitter.on('sendProgress', function ( connection) {
    var status = {};
    status.details = {};
    var progress = 0;
    var nodes = ["sparkTextFileSource", "sparkObjectFileSink", "javaObjectFileSource", "javaFlatMapOperator", "javaObjectFileSink"];
    var id = setInterval(progressing, 1000);
    function progressing() {
      if(progress > 100){
        clearInterval(id);
      }else{
        status.overall = progress;
        for(var i=0; i< nodes.length; i++){
          if(progress / 20 > i){
            status.details[nodes[i]] = 100;
          }else if(progress / 20 == i){
            status.details[nodes[i]] = progress;
          }else{
            if(progress - (20 * i) > 0)
              status.details[nodes[i]] = progress - (20 * i);
            else
              status.details[nodes[i]] = 0;
          }
        }
        connection.send(JSON.stringify(status))
        progress+=20;
      }
    }
  });

  console.log((new Date()) + ' Connection accepted.');

  connection.on('message', function(message) {
    console.log(message);
    eventEmitter.emit("sendProgress", connection);
  });

  connection.on('close', function(connection) {
    console.log("Closed");
  });

  connection.on('error', function(err) {
    console.log("errors: ",err);
  });

});