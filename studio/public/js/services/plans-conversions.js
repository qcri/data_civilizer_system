appServices.service('plansConversions', function() {
  var executionPlan = undefined;

  var set = function(model) {
      executionPlan = model;
  };

  var get = function(){
      return executionPlan;
  };

  var getPlanFromNodes = function(nodes) {
    var operators = [];
    var sinks = [];
    var operator;
    var edge;
    var outs = {};
    for(var node in nodes){
        operator = {};
        operator["name"] = nodes[node].name;
        operator["java_class"] = nodes[node].java_class;
        operator["x"] = nodes[node].x;
        operator["y"] = nodes[node].y;
        operator["color"] = nodes[node].color;
        operator["parameters"] = nodes[node].parameters;
        operator["selectedConstructor"] = nodes[node].selectedConstructor;
        operator["isBroadCast"] = nodes[node].isBroadCast;
        operator["features"] = nodes[node].features;
        operator["baseurl"] = nodes[node].baseurl;
        operator["type"] = nodes[node].type;
        operator["connects_to"] = {}
        operator["broadcasts_to"] = {}
        operator["np_inputs"] = nodes[node].tops.length;
        operator["np_outputs"] = nodes[node].buttons.length;
        if(nodes[node].buttons.length == 0){
            sinks.push(nodes[node].name);
        }
        for(var port in nodes[node].nextNodes){
            operator["connects_to"][port] = [];
            for(var nextNode in nodes[node].nextNodes[port]){
              edge = {};
              edge[nodes[node].nextNodes[port][nextNode].name] = nodes[node].nextNodes[port][nextNode].port;
              operator["connects_to"][port].push(edge);
            }
        }
        for(var port in nodes[node].broadcastNodes) {
            operator["broadcasts_to"][port] = [];
            for(var nextNode in nodes[node].broadcastNodes[port]){
              edge = {};
              edge[nodes[node].broadcastNodes[port][nextNode].name] = nodes[node].broadcastNodes[port][nextNode].port;
              operator["broadcasts_to"][port].push(edge);
            }
        }
        operators.push(operator);
    }
    var plan = {};
    plan["sink_operators"] = sinks;
    plan["operators"] = operators;
    return plan;
  };

  var getNodeFromView =  function (model) {
    var nodes = {}
    var buttons = {};
    var tops = {};
    var broadcasts = {};
    for (var node in model["nodes"]){
      var connectors = model["nodes"][node]["connectors"];
      var top_ids=[], button_ids=[], broadcast_ids = [];
      for (var i = 0; i < connectors.length; i++) {
         if(connectors[i].type == "topConnector" && !connectors[i].broadcast){
          top_ids.push(connectors[i].id);
         }else if (connectors[i].type == "bottomConnector"){
          button_ids.push(connectors[i].id);
         }else{
          broadcast_ids.push(connectors[i].id);
         }
      }
      nodes[model["nodes"][node]["id"]] = createNode(
            model["nodes"][node]["id"],
            model["nodes"][node]["name"],
            model["nodes"][node]["java_class"],
            model["nodes"][node]["x"],
            model["nodes"][node]["y"],
            model["nodes"][node]["color"],
            top_ids,
            button_ids,
            broadcast_ids,
            model["nodes"][node]["parameters"],
            model["nodes"][node]["selectedConstructor"],
            model["nodes"][node]["isBroadCast"],
            model["nodes"][node]["features"],
            model["nodes"][node]["baseurl"],
            model["nodes"][node]["type"]
        );
      top_ids.map(function(top_id) {
         tops[top_id] = nodes[model["nodes"][node]["id"]];
      });
      button_ids.map(function(button_id) {
         buttons[button_id] = nodes[model["nodes"][node]["id"]];
      });
      broadcast_ids.map(function (broadcast_id) {
         broadcasts[broadcast_id] = nodes[model["nodes"][node]["id"]];
      });
    }
    var edges = model["edges"];
    for(var node in nodes){
      nodes[node].addNodes(tops, broadcasts, edges);
    }
    return nodes;
  };

  var createNode =  function (id, name, java_class, x, y, color, tops, buttons, broadcasts, parameters, selectedConstructor, isBroadCast, features, baseurl, type) {
    var Node = function (id, name, java_class, x, y, color, tops, buttons, broadcasts, parameters, selectedConstructor, isBroadCast, features, baseurl, type){
      this.id = id;
      this.y = y;
      this.x = x;
      this.color = color;
      this.java_class = java_class;
      this.name = name;
      this.tops = tops;
      this.buttons = buttons;
      this.broadcasts = broadcasts;
      this.parameters = parameters;
      this.selectedConstructor = selectedConstructor;
      this.isBroadCast = isBroadCast;
      this.features = features;
      this.baseurl = baseurl;
      this.type = type;
      this.broadcastNodes = {};
      this.nextNodes = {};
      this.nextNodesEdges = {};
      this.addNodes = function(topss, broadcastss, edges){
        var index;
        for (var edge in edges){
          index = this.buttons.indexOf(edges[edge].source);
          if(index != -1){
            if(topss.hasOwnProperty(edges[edge].destination)){
              this.nextNodes[index] = this.nextNodes[index] || [];
              this.nextNodes[index].push({name: topss[edges[edge].destination].name, port: topss[edges[edge].destination].tops.indexOf(edges[edge].destination)});
            }
            else if(broadcastss.hasOwnProperty(edges[edge].destination)){
              this.broadcastNodes[index] = this.broadcastNodes[index] || [];
              this.broadcastNodes[index].push({name: broadcastss[edges[edge].destination].name, port: broadcastss[edges[edge].destination].broadcasts.indexOf(edges[edge].destination)});
            }
           }
        }
      };
    };
    return new Node(id, name, java_class, x, y, color, tops, buttons, broadcasts, parameters, selectedConstructor, isBroadCast, features, baseurl, type);
  };

  var getNodesFromDB =  function (plan) {
    var nodes = {};
    var gid = 1;
    var eid = 0;
    for(var operator in plan["operators"]){
      var node;
      var name = plan["operators"][operator]["name"];
      var java_class = plan["operators"][operator]["java_class"];
      var x = plan["operators"][operator]["x"];
      var y = plan["operators"][operator]["y"];
      var parameters = plan["operators"][operator]["parameters"];
      var selectedConstructor = plan["operators"][operator]["selectedConstructor"];
      var isBroadCast = plan["operators"][operator]["isBroadCast"];
      var features = plan["operators"][operator]["features"];
      var baseurl = plan["operators"][operator]["baseurl"];
      var type = plan["operators"][operator]["type"];
      var color = plan["operators"][operator]["color"];
      var np_inputs = plan["operators"][operator]["np_inputs"];
      var np_outputs = plan["operators"][operator]["np_outputs"];
      var id = gid++;
      node = createNode(id, name, java_class, x, y, color, [], [], [], parameters, selectedConstructor, isBroadCast, features, baseurl, type);
      for(var connectors in plan["operators"][operator]["connects_to"]){
        var via;
        var nodeName;
        for (var i = 0; i < plan["operators"][operator]["connects_to"][connectors].length; i++) {
         for(var type in plan["operators"][operator]["connects_to"][connectors][i]){
            if(type == "via"){
              via = plan["operators"][operator]["connects_to"][connectors][i][type];
            }else{
              node.nextNodes[connectors] = node.nextNodes[connectors] || [];
              node.nextNodes[connectors].push({name: type, port: plan["operators"][operator]["connects_to"][connectors][i][type]});
              nodeName = type;
            }
          }
          node.nextNodesEdges[nodeName] = via;
        }
      }
      for(var connectors in plan["operators"][operator]["broadcasts_to"]){
         for (var i = 0; i < plan["operators"][operator]["broadcasts_to"][connectors].length; i++) {
           for(var type in plan["operators"][operator]["broadcasts_to"][connectors][i]){
              if(type == "via"){
                 via = plan["operators"][operator]["broadcasts_to"][connectors][i][type];
              }else{
                node.broadcastNodes[connectors] = node.broadcastNodes[connectors] || [];
                node.broadcastNodes[connectors].push({name: type, port: plan["operators"][operator]["broadcasts_to"][connectors][i][type]});
              }
            }
         }
      }
      for (var i = 0; i < np_inputs; i++) {
        node.tops.push(--eid);
      }
      for (var i = 0; i < np_outputs; i++) {
        node.buttons.push(--eid);
      }

      if(isBroadCast){
        node.broadcasts.push(--eid);
        node.broadcasts.push(--eid);
      }

      nodes[name] = node;
   }
   return nodes;
  };

  var getViewedJson = function (nodes) {
    var viewNodes = [];
    var edges = [];
    var node, edge;
    var eid = 0;
    for(var name in nodes){
      node = {};
      node["id"] = nodes[name].id;
      node["name"] = nodes[name].name;
      node["color"] = nodes[name].color;
      node["x"] = nodes[name].x;
      node["y"] = nodes[name].y;
      node["java_class"] = nodes[name].java_class;
      node["parameters"] = nodes[name].parameters;
      node["selectedConstructor"] = nodes[name].selectedConstructor;
      node["isBroadCast"] = nodes[name].isBroadCast;
      node["features"] = nodes[name].features;
      node["baseurl"] = nodes[name].baseurl;
      node["type"] = nodes[name].type;
      node["connectors"] = [];
      nodes[name].tops.map(function(top_id) {
        node["connectors"].push({id: top_id, type: "topConnector"});
      });
      nodes[name].buttons.map(function(button_id) {
        node["connectors"].push({id: button_id, type: "bottomConnector"});
      });
      nodes[name].broadcasts.map(function(broadcast_id) {
        node["connectors"].push({id: broadcast_id, type: "topConnector", broadcast: true});
      });

      for(var connector in nodes[name].nextNodes){
          for (var i = 0; i < nodes[name].nextNodes[connector].length; i++) {
            edge = {};
            edge["source"] = nodes[name].buttons[connector];
            edge["destination"] = nodes[nodes[name].nextNodes[connector][i].name].tops[nodes[name].nextNodes[connector][i].port];
            edges.push(edge);
          }
      }
      for(var connector in nodes[name].broadcastNodes){
          for (var i = 0; i < nodes[name].broadcastNodes[connector].length; i++) {
            edge = {};
            edge["source"] = nodes[name].buttons[connector];
            edge["destination"] = nodes[nodes[name].broadcastNodes[connector][i].name].broadcasts[nodes[name].broadcastNodes[connector][i].port];
            edges.push(edge);
          }
      }
      viewNodes.push(node);
    }
    return {"nodes": viewNodes, "edges": edges};
  };

  var getViewFromExec =  function (execPlan, xstep, ystep, color){
    const COLORS = color;
    const XStep = xstep, YStep = ystep;
    var plan = {};
    var index = 0;
    plan["operators"] = [];

    for(var operators in execPlan["stages"]){
      plan["operators"] = plan["operators"].concat(execPlan["stages"][operators]["operators"]);
    }

    var nodes = getNodesFromDB(plan);
    var sorted = {};
    var onprogress = [];
    var visit = function (parentName,node, sorted, level) {
      if(onprogress.indexOf(node.name) != -1)
        return;
      onprogress.push(node.name);
      for (var i = 0; i < node.nextNodes.length; i++) {
        visit(node.name,node.nextNodes[i], sorted, level+1);
      }
      onprogress.splice(onprogress.indexOf(node.name), 1);
      sorted[node.name] = sorted[node.name] || {};
      sorted[node.name]["level"] = sorted[node.name]["level"] || -1;
      if(sorted[node.name]["level"] < level){
        sorted[node.name]["level"] = level;
        sorted[node.name]["parent"] = parentName;
      }
    }
    for(var name in nodes){
      visit(null,nodes[name], sorted, 0);
    }
    var execGraph = {nodes:{}, edges: []}
    var node;
    var levels = [], levelCount = [];
    for(var name in sorted){
      levelCount[sorted[name]["level"]] = levelCount[sorted[name]["level"]] + 1 || 1;
    }

    for (var i = 0; i < execPlan["stages"].length; i++) {
      var color = COLORS[execPlan["stages"][i].platform];
      for (var j = 0; j < execPlan["stages"][i]["operators"].length; j++) {
         node = execPlan["stages"][i]["operators"][j];
         execGraph.nodes[node.name] = {};
         execGraph.nodes[node.name]["color"] = color;

         execGraph.nodes[node.name]["x"] = sorted[node.name]["level"] * XStep;
         levels[sorted[node.name]["level"]] = levels[sorted[node.name]["level"]] + 1 || 0;

         if(levelCount[sorted[node.name]["level"]] < 2)
            execGraph.nodes[node.name]["y"] = YStep / 2;
         else{
            execGraph.nodes[node.name]["y"] = levels[sorted[node.name]["level"]] * YStep;
         }

         for(var next in nodes[node.name].nextNodes){
          for (var k = 0; k < nodes[node.name].nextNodes[next].length; k++) {
            execGraph.edges.push({source: node.name,
              target: nodes[node.name].nextNodes[next][k].name,
              type: nodes[node.name].nextNodesEdges[nodes[node.name].nextNodes[next][k].name]
            });
          }
         }
      }
    }

    for(var name in execGraph.nodes){
      if(sorted[name].parent && nodes[sorted[name].parent].nextNodes.length == 1){
         execGraph.nodes[name].y = execGraph.nodes[sorted[name].parent].y;
      }
    }

    return execGraph;
  };

  return {
    get: get,
    set: set,
    convertToPlan: getPlanFromNodes,
    getViewedJson: getViewedJson,
    getViewFromExec: getViewFromExec,
    getNodeFromView: getNodeFromView,
    getNodesFromDB: getNodesFromDB
  };

});
