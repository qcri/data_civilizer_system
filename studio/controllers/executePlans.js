var Proxy = require('../helpers/proxy');
proxy = new Proxy();
var express = require('express');
var router = express.Router();
http = require('http');
fs = require('fs');
request = require('request-json');

const uuid = require('uuid');

var client = request.createClient(process.env.API_SERVER_URL);

// Independent execution engine for gemPlans -- not yet implemented

const STATE_WAITING = 0;
const STATE_ACTIVE  = 1;
const STATE_USER    = 2;
const STATE_LOOP    = 3;
const STATE_DONE    = 4;
const STATE_ERROR   = 5;

var planStatus = {};

function ExecutePlans_gem(gemPlan) {
}

function ExecutePlans_rheem(rheemPlan) {
  // Create local associative array of plan operators indexed by op.name
  console.log("");
  console.log("Execute rheem plan");
  console.log(JSON.stringify(rheemPlan, null, 4));

  // Assign a unique id for this plan execution
  // Create a status object to track and report plan execution
//var run_id = ("0000000" + Math.floor(Math.random() * 2147483647).toString(16)).slice(-8);
  var run_id = uuid.v1();
  var ops = {};
  console.log("run_id: " + run_id);
  var status = planStatus[run_id] = {
    "run_id": run_id,
    "state": STATE_ACTIVE,
    "op_status": ops
  };

  console.log("");
  console.log("Info: Initializing ops index and depends_on lists...");
  for(var op of rheemPlan.operators) {
    console.log("..adding op.name: " + op.name);
    ops[op.name] = op;
    op.run_id = run_id;
    op.state = STATE_WAITING;
    op.inputs = [];
    op.depends_on = [];
    op.links_to = [];
  }

  console.log("");
  console.log("Info: Building depends_on lists...");
  var inits = [];
  for(var key in ops) {
    var op = ops[key];
    console.log("..building depends_on for: " + op.name);
    if("connects_to" in op) {
      for(var onum in op.connects_to) {
        var links = op.connects_to[onum];
        op.links_to[onum] = {};
        for(var link of links) {
          for(var name in link) {
            console.log("....link name: " + name);
            ops[name].depends_on.push(op.name);
            op.links_to[onum][name] = link[name];
          }
        }
      }
    }
    if(op.np_inputs == 0) {
      inits.push(op.name);
    }
  }

  // Build "depends_on" array for each operator element to aid execution

  var active = 0;

  executePlan(inits);
  console.log("Past initial executePlans() call");

  function executePlan(keys) {
    console.log("executePlan(" + keys + ") - Looking for operators to execute");
    for(var key of keys) {
      var op = ops[key];
      console.log("Checking op: " + op.name);
      if(op.state == STATE_WAITING) {
        var waiting = op.depends_on.length;
        for(var name of op.depends_on) {
          if(ops[name].state == STATE_DONE) {
            waiting--;
          }
        }
        if(!waiting) {
          executeNode(op);
          // return; // uncomment to serialize execution
        }
      }
    }

    console.log("Activity: " + active);
    if(!active) {
      // No activity detected -- execution is done
      for(var name in ops) {
        status.state = Math.max(status.state, ops[name].state);
        if(ops[name].state == STATE_ERROR) {
          status.error = ops[name].error;
        }
      }
      setTimeout(clearStatus, 1 * 60 * 60 * 1000); // cache results for 1 hour
    }
  }

  function executeNode(op) {
    active++;
    console.log("Executing: " + op.name);
    op.state = STATE_ACTIVE;
    var url = "http://apis:8089/rheem/plan_exec_op";
    console.log("Sending: " + JSON.stringify(op, null, 4));
    client.post(url, op, function(err, data) {
      active--;
      var next_keys = [];
      if(err) {
        // HTTP errors connecting to opertor service
        console.log("HTTP error from POST " + url + " for " + op.name);
        console.log(JSON.stringify(err, null, 4));
        op.state = STATE_ERROR;
        op.error = err;
      } else {
        console.log("Status "  + data.statusCode + " from " + op.name);
        if(data.statusCode != 200) {
          op.state = STATE_ERROR;
          op.error = data.body;
        } else {
          if(("error" in data.body) && data.body.error) {
            console.log("Error from " + op.name + ": " + data.body.error);
            op.state = STATE_ERROR;
            op.error = data.body;
          } else {
            op.state = STATE_DONE;
            op.output = data.body;

            // Pass output(s) onto all 'next' operators
            op.outputs = data.body;
            if(op.outputs.toString != "[object Array]") {
              op.outputs = [op.outputs];
            }
            for(var onum = 0; onum < op.links_to.length; onum++) {
              next_keys = next_keys.concat(Object.keys(op.links_to[onum]));
              for(var op_key in op.links_to[onum]) {
                var next_op = ops[op_key];
                next_op.inputs[op.links_to[onum][op_key]] =
                  onum < op.outputs.length ? op.outputs[onum] : {};
              }
            }
          }
        }
      }
      executePlan(next_keys);
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
