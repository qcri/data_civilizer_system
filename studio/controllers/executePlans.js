var Proxy = require('../helpers/proxy');
proxy = new Proxy();
var express = require('express');
var router = express.Router();
http = require('http');
fs = require('fs');
request = require('request-json');

var client = request.createClient(process.env.API_SERVER_URL);

// Independent execution engine for gemPlans -- not yet implemented

const STATE_WAITING = 0;
const STATE_ACTIVE  = 1;
const STATE_DONE    = 2;
const STATE_ERROR   = 3;

var planStatus = {};

function ExecutePlans(gemPlan) {
    var run_id = Math.floor(Math.random() * (1 << 32)).toString(16);
    planStatus[run_id] = {
        "run_id": run_id,
        "state": STATE_ACTIVE
    };
    for(var op in gemPlan) {
        planStatus[op.id] = {
            "state": STATE_WAITING,
            "data": null
        };
    }

    executePlan();

    function executePlan() {
        var active = 0;
        for(var op in gemPlan) {
            switch(planStatus) {
                case STATE_WAITING:
                    var waiting = op.prev.length;
                    for(var index of op.prev) {
                        if(planStatus[index].state == STATE_DONE) {
                            waiting--;
                        }
                    }
                    if(waiting) {
                        break;
                    }
                    executeNode(op);
                case STATE_ACTIVE:
                    active++;
                default:
                    break;
            }
        }
        if(!active) {
            planStatus.state = STATE_DONE;
            setTimeout(clearStatus, 1 * 60 * 60 * 1000); // cache results for 1 hour
        }
    }

    function executeNode(op) {
        planStatus[op.id].state = STATE_ACTIVE;
        http.post({url:"http://apis:8089", data:""}, function(err, data) {
            if(err) {
                planStatus[op.id].state = STATE_ERROR;
                planStatus[op.id].error = err;
            } else {
                planStatus[op.id].state = STATE_DONE;
                planStatus[op.id].data = data;

                // Code to pass results on to 'next' operators goes here
            }
            executePlan();
        });
    }

    function clearStatus() {
        delete planStatus[run_id];
    }

    return JSON.stringify(planStatus);
}

router.post('/rheem_plans', function(req, res) {
  console.log("POST " + process.env.API_SERVER_URL + "/rheem/plan_plans @ " + new Date().toISOString());
  console.log("sent", JSON.stringify(req.body, null, 4));

  var data = req.body;
  client.post('/rheem/rheem_plans', data, function(err, reshttp, body) {
    console.log("return " , body);
    return res.json(body)
  });
});

router.post('/plan_executions', function(req, res) {
  console.log("POST " + process.env.API_SERVER_URL + "/rheem/plan_executions @ " + new Date().toISOString());
  console.log(JSON.stringify(req.body, null, 4));

  var data = req.body;
  client.post('/rheem/plan_executions', data, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});

router.get('/latest_plan_execution', function(req, res){
  console.log("GET " + process.env.API_SERVER_URL + "/rheem/latest_run @ " + new Date().toISOString());

  client.get('/rheem/latest_run', function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});

router.get('/progress', function(req, res){
  console.log("GET " + process.env.API_SERVER_URL + "/rheem/progress?run_id=" + req.query.run_id + " @ " + new Date().toISOString());
  console.log("dsndnjs");
  client.get('/rheem/progress?run_id='+ req.query.run_id, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});


module.exports = router;
