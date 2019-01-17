var Proxy = require('../helpers/proxy');
proxy = new Proxy();
var express = require('express');
var router = express.Router();
http = require('http');
fs = require('fs');
request = require('request-json');

const uuid = require('uuid');

var client = request.createClient(process.env.API_SERVER_URL);
var clients = {};

// Independent execution engine for gemPlans -- not yet implemented

const STATE_WAITING = 0;
const STATE_BLOCKED = 1;
const STATE_ACTIVE  = 2;
const STATE_USER    = 3;
const STATE_LOOP    = 4;
const STATE_DONE    = 5;
const STATE_ERROR   = 6;

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
  var run_id = uuid.v1();
  var ops = {};
  console.log("run_id: " + run_id);
  var status = planStatus[run_id] = {
    "run_id": run_id,
    "state": STATE_ACTIVE,
    "op_status": ops,
    "serial": {},
    "blocked": {}
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
    for(var key of Object.keys(status.blocked).concat(keys)) {
      var op = ops[key];
      console.log("Checking op: " + op.name);
      if(op.state < STATE_ACTIVE) {
        var waiting = op.depends_on.length;
        for(var name of op.depends_on) {
          if(ops[name].state == STATE_DONE) {
            waiting--;
          }
        }
        if(!waiting) {
          // New default is to serialize execution of instances of the same
          // operator unless they explicitly specify that they support
          // parallelization.
          if(!(op.java_class in status.serial) || (op.features && op.features.parallel)) {
            if(op.name in status.blocked) {
              delete status.blocked[op.name];
            }
            executeNode(op);
          } else {
            status.blocked[op.name] = true;
            op.state = STATE_BLOCKED;
          }
          // return; // uncomment to serialize plan execution
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
    if(!op.features || !op.features.parallel) {
      status.serial[op.java_class] = op.name;
    }
    console.log("Executing: " + op.name);
    op.state = STATE_ACTIVE;
    var url = op.baseurl + "/rheem/plan_exec_op";
    console.log("POST " + url + " @ " + new Date().toISOString());
    if(!(op.baseurl in clients)) {
      clients[op.baseurl] = request.createClient(op.baseurl);
    }
    console.log("Sending: " + JSON.stringify(op, null, 4));
    clients[op.baseurl].post(url, op, function(err, data) {
      active--;
      if((op.java_class in status.serial) && (status.serial[op.java_class] == op.name)) {
        delete status.serial[op.java_class];
      }
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
    var run_id = req.query.run_id;
    console.log("GET /progress?run_id=" + run_id);

if(0) {
    //
    // Old method -
    // -- optimized to make a single progress call to the backend
    //    apis container by specifying mulitple "name=" variables
    //    in the query string.
    // -- does not support operators coming from multiple backends,
    //    which would be useful to support future operators running
    //    in separte containers
    //

    var names = "";
    var op_status = planStatus[run_id].op_status;
    for(var name in op_status) {
      if(op_status[name].state == STATE_ACTIVE) {
        console.log("op_status[" + name + "]:");
        console.log(op_status[name]);
        if(op_status[name].features && op_status[name].features.progress) {
          names += "&name=" + name;
        }
      }
    }

    if(names) {
      console.log("GET " + process.env.API_SERVER_URL + "/rheem/progress?run_id=" + run_id + names + " @ " + new Date().toISOString());
      client.get('/rheem/progress?run_id='+ run_id + names, function(err, reshttp, body) {
        console.log(body);
        for(var name in body) {
          if(body[name].progress) {
            op_status[name].progress = body[name].progress;
          }
        }
        return res.json(planStatus[run_id]);
      });
    } else {
      return res.json(planStatus[run_id]);
    }

} else {

    //
    // New method -
    // -- Groups operators that support advanced progress reporting
    //    by 'baseurl' property (which identifies the api container
    //    of the operator)
    // -- Makes one call per backend container and aggregates results
    //

    var names = {};
    var url_cnt = 0;
    var op_status = planStatus[run_id].op_status;
    for(var name in op_status) {
      if(op_status[name].state == STATE_ACTIVE) {
        console.log("op_status[" + name + "]:");
        console.log(op_status[name]);
        if(op_status[name].features && op_status[name].features.progress) {
          var baseurl = op_status[name].baseurl;
          if(!(baseurl in names)) {
            names[baseurl] = "";
            url_cnt++;
          }
          names[baseurl] += "&name=" + name;
        }
      }
    }

    if(url_cnt) {
      for(var baseurl in names) {
        console.log("GET " + baseurl + "/rheem/progress?run_id=" + run_id + names[baseurl] + " @ " + new Date().toISOString());
        if(!(baseurl in clients)) {
          clients[baseurl] = request.createClient(baseurl);
        }
        clients[baseurl].get('/rheem/progress?run_id='+ run_id + names[baseurl], function(err, reshttp, body) {
          console.log(body);
          if(body) {
            for(var name in body) {
              if(body[name].progress) {
                op_status[name].progress = body[name].progress;
              }
            }
          }
          if(--url_cnt == 0) {
            return res.json(planStatus[run_id]);
          }
        });
      }
    } else {
      return res.json(planStatus[run_id]);
    }

}

    return;
  }

  console.log("GET " + process.env.API_SERVER_URL + "/rheem/progress?run_id=" + req.query.run_id + " @ " + new Date().toISOString());

  console.log("dsndnjs");
  client.get('/rheem/progress?run_id='+ req.query.run_id, function(err, reshttp, body) {
    console.log(body);
    return res.json(body)
  });
});


module.exports = router;
