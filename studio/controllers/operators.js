var Proxy = require('../helpers/proxy');
proxy = new Proxy();

var request = require('request-json');
var clients = {};
var apis = process.env.API_SERVER_URL.split(";");
for(var baseurl of apis) {
  clients[baseurl] = request.createClient(baseurl);
}

function Operator() {
}

Operator.getByType = function(req, res){
if(0) {
  //
  // Old method -
  // -- simple proxy to apis container to get operators.json
  //

  var target = req.method + " " + process.env.API_SERVER_URL + "/rheem" + req.url;
  console.log(target + " @ " + new Date().toISOString());
  // console.log(req.getallheaders);
  // console.log(JSON.stringify(req.headers, null, 4));

  // Catch proxy errors if the backend is down...
  proxy.proxy.on('error', function (err, req, res) {
    console.log("Proxy error on " + target + "\n" + JSON.stringify(err, null, 4));
    return res.end();
  });

  proxy.forwardRequest(req, res);
} else {
  //
  // New method -
  // -- API_SERVER_URL can list multiple backend api servers delimited by ";"
  // -- All backends are queried and results are aggregated
  // -- Each operator is tagged with the 'baseurl' of its associated backend
  //

  var requests = 0;
  var operators = { "operators": [] };
  for(var baseurl in clients) {
    var target = req.method + " " + baseurl + "/rheem" + req.url;
    console.log(target + " @ " + new Date().toISOString());
    requests++;

    clients[baseurl].get("/rheem" + req.url, function(err, reshttp, body) {
      console.log(body);
      if(body && ("operators" in body) && Array.isArray(body.operators)) {
        for(var operator of body.operators) {
          operator.baseurl = baseurl;
        }
        operators.operators = operators.operators.concat(body.operators);
      }
      if(--requests == 0) {
        console.log("Operator.getByType returning:\r\n" + JSON.stringify(operators, null, 4));
        return res.json(operators);
      }
    });
  }
}
};

module.exports = Operator;
