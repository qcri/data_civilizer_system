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

function ExecutePlans_gem(gemPlan) {
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

function ExecutePlans_rheem(rheemPlan) {
  // Create local associative array of plan operators indexed by op.name
  console.log("Info: Building ops index and depends_on lists");
  var ops = {};
  for(var op of rheemPlan.operators) {
    console.log("..adding op.name: " + op.name);
    ops[op.name] = op;
    op.depends_on = [];
    op.links_to = [];
  }

  // Build "depends_on" array for each operator element to aid execution
  var inits = [];
  for(var op of rheemPlan.operators) {
    console.log("..building depends_on for: " + op.name);
    if("connects_to" in op) {
      for(var inum in op.connects_to) {
        var links = op.connects_to[inum];
        for(var link of links) {
          for(var name in link) {
            console.log("....link name: " + name);
            ops[name].depends_on.push(op.name);
            op.links_to.push(name);
          }
        }
      }
    }
    if(op.depends_on.length == 0) {
      inits.push(op.name);
    }
  }

  // Assign a unique id for this plan execution
  // Create a status object to track and report plan execution
  var run_id = ("0000000" + Math.floor(Math.random() * 2147483647).toString(16)).slice(-8);
  console.log("run_id: " + run_id);
  var status = planStatus[run_id] = {
    "run_id": run_id,
    "state": STATE_ACTIVE,
    "op_stat": {}
  };
  for(var name in ops) {
    status.op_stat[name] = {
      "state": STATE_WAITING,
      "data": null
    };
  }

  var active = 0;

  executePlan(inits);
  console.log("Past initial executePlans() call");

  function executePlan(keys) {
    console.log("executePlan() - Looking for operators to execute");
//  var active = 0;
    for(var key of keys) {
      var op = ops[key];
      console.log("Checking op: " + op.name);
      switch(status.op_stat[op.name].state) {
        case STATE_WAITING:
          var waiting = op.depends_on.length;
          for(var name of op.depends_on) {
            if(status.op_stat[name].state == STATE_DONE) {
              waiting--;
            }
          }
          if(waiting) {
            break;
          }
          executeNode(op);
          // return; // uncomment to serialize execution
        case STATE_ACTIVE:
//        active++;
        default:
          break;
      }
    }
    console.log("Activity: " + active);
    if(!active) {
      status.state = STATE_DONE;
      setTimeout(clearStatus, 1 * 60 * 60 * 1000); // cache results for 1 hour
    }
  }

  function executeNode(op) {
    active++;
    console.log("Executing: " + op.name);
    status.op_stat[op.name].state = STATE_ACTIVE;
//  http({ method:"POST:, url:"http://apis:8089/rheem/plan_exec_op", data:JSON.stringify(op) }, function(err, data) {
    client.post("http://apis:8089/rheem/plan_exec_op", JSON.stringify(op), function(err, data) {
      active--;
      if(err) {
        console.log("Error from: " + op.name);
        console.log("..error=" + JSON.stringify(err));
        status.op_stat[op.name].state = STATE_ERROR;
        status.op_stat[op.name].error = err;
      } else {
        console.log("Success from: " + op.name);
        status.op_stat[op.name].state = STATE_DONE;
        status.op_stat[op.name].data = data;

        // Code to pass results on to 'next' operators goes here
      }
      executePlan(op.links_to);
    });
    console.log("After client post to apis");
  }

  function clearStatus() {
    delete planStatus[run_id];
  }

  return status;
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

  if("operators" in req.body) {
    var plan = req.body;
    var active = -1;
    for(var index = 0; index < plan.operators.length; index++) {
      if(plan.operators[index].parameters.param1 == "y") {
        active = index;
        break;
      }
    }
    if(active == -1) {
     console.log("Info: no operator selected - calling ExecutePlans");
       var s = ExecutePlans_rheem(req.body);
      console.log("ExecutePlans returned " + s);
      return res.json(s); // ExecutePlans_rheem(req.body);
    }
  }

  var data = req.body;
  client.post('/rheem/plan_executions', data, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});

router.get('/latest_plan_execution', function(req, res) {
  console.log("GET " + process.env.API_SERVER_URL + "/rheem/latest_run @ " + new Date().toISOString());

  client.get('/rheem/latest_run', function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});

router.get('/progress', function(req, res) {
  if(req.query.run_id in planStatus) {
    console.log("GET /progress?run_id=" + req.query.run_id);
    return res.json(planStatus[req.query.run_id]);
  }

  console.log("GET " + process.env.API_SERVER_URL + "/rheem/progress?run_id=" + req.query.run_id + " @ " + new Date().toISOString());

  console.log("dsndnjs");
  client.get('/rheem/progress?run_id='+ req.query.run_id, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});


module.exports = router;
