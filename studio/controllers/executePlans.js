var Proxy = require('../helpers/proxy');
proxy = new Proxy();
var express = require('express');
var router = express.Router();
http = require('http');
fs = require('fs');
request = require('request-json');

var client = request.createClient(process.env.API_SERVER_URL);

function ExecutePlans() {

}

router.post('/rheem_plans', function(req, res) {
  console.log("sent" ,JSON.stringify(req.body));

  var data = req.body;
  client.post('/rheem/rheem_plans', data, function(err, reshttp, body) {
    console.log("return " , body);
    return res.json(body)
  });
});

router.post('/plan_executions', function(req, res) {
  console.log(JSON.stringify(req.body));

  var data = req.body;
  client.post('/rheem/plan_executions', data, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});

router.get('/latest_plan_execution', function(req, res){

  client.get('/rheem/latest_run', function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});

router.get('/progress', function(req, res){
  console.log("dsndnjs");
  client.get('/rheem/progress?run_id='+ req.query.run_id, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});


module.exports = router;
