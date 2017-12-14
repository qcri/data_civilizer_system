var httpProxy = require('http-proxy');
var http = require("http");

function WebSocket (port) {
  var server = http.createServer(function(request, response) {}).listen(port);
  var websocket = new httpProxy.createProxyServer({
    target: process.env.SOCKET_SERVER_ENDPOINT,
    changeOrigin: true,
    ws: true,
    secure: true
  });
  server.on('upgrade', function (req, socket, head) {
    websocket.ws(req, socket, head);
  });
}

module.exports = WebSocket;
