function Graph () {}

Graph.prototype = {

   createNode: function (id, name, java_class, x, y, color, top, button, parameters) {
      var Node = function (id, name, java_class, x, y, color, top, button, parameters){
         this.id = id;
         this.y = y;
         this.x = x;
         this.color = color;
         this.java_class = java_class;
         this.name = name;
         this.top = top;
         this.button = button;
         this.parameters = parameters;
         this.nextNodes = [];
         this.nextNodesEdges = [];
         this.addNodes = function(tops, edges){
            for (edge in edges){
               if(edges[edge]["source"] == this.button){
                  this.nextNodes.push(tops[edges[edge]["destination"]]);
               }
            }
         };
      };
      return new Node(id, name, java_class, x, y, color, top, button, parameters);
   },

   getPlanFromNodes: function (nodes) {
      var operators = [];
      var sinks = [];
      var operator;
      var edge;
      var outs = {};
      for(node in nodes){
         operator = {};
         operator["name"] = nodes[node].name;
         operator["java_class"] = nodes[node].java_class;
         operator["x"] = nodes[node].x;
         operator["y"] = nodes[node].y;
         operator["color"] = nodes[node].color;
         operator["parameters"] = nodes[node].parameters;
         operator["connects_to"] = {}
         if(nodes[node].nextNodes.length == 0){
            sinks.push(nodes[node].name);
         }
         for(nextNode in nodes[node].nextNodes){
            operator["connects_to"][nextNode] = [];
            edge = {};
            outs[nodes[node].nextNodes[nextNode].name] = outs[nodes[node].nextNodes[nextNode].name] + 1 || 0
            edge[nodes[node].nextNodes[nextNode].name] = outs[nodes[node].nextNodes[nextNode].name]
            operator["connects_to"][nextNode].push(edge);
         }
         operators.push(operator);
      }
      var plan = {};
      plan["sink_operators"] = sinks;
      plan["operators"] = operators;
      return plan;
   },

   getViewedJson: function (fullNodes) {
      var nodes = [];
      var edges = [];
      var node, edge;
      for(name in fullNodes["nodes"]){
         node = {};
         node["id"] = fullNodes["nodes"][name].id;
         node["name"] = fullNodes["nodes"][name].name;
         node["color"] = fullNodes["nodes"][name].color;
         node["x"] = fullNodes["nodes"][name].x;
         node["y"] = fullNodes["nodes"][name].y;
         node["java_class"] = fullNodes["nodes"][name].java_class;
         node["parameters"] = fullNodes["nodes"][name].parameters;
         if(fullNodes["nodes"][name].top != null){
            node["connectors"] = [];
            node["connectors"].push({"type": "topConnector", "id": fullNodes["tops"][node["name"]]});
         }

         if(fullNodes["nodes"][name].button != null){
            node["connectors"] = node["connectors"] || [];
            node["connectors"].push({"type": "bottomConnector", "id": fullNodes["buttons"][node["name"]]});
         }

         for(next in fullNodes["nodes"][name].nextNodes){
            edge = {};
            edge["source"] = fullNodes["nodes"][name].button
            edge["destination"] = fullNodes["nodes"][name].nextNodes[next].top;
            edges.push(edge);
         }
         nodes.push(node);
      }
      return {"nodes": nodes, "edges": edges};
   },

   getNodeFromView: function (json) {
      var nodes = {}
      var buttons = {};
      var tops = {};
      for (node in json["nodes"]){
         var connectors = json["nodes"][node]["connectors"];
         var top_id=null, button_id=null;
         if(connectors[0]["type"] == "bottomConnector"){
            button_id = connectors[0]["id"];
         }else if(connectors.length > 1 && connectors[1]["type"] == "bottomConnector"){
            button_id = connectors[1]["id"];
         }
         if(connectors[0]["type"] == "topConnector"){
            top_id = connectors[0]["id"];
         }else if(connectors.length > 1 && connectors[1]["type"] == "topConnector"){
            top_id = connectors[1]["id"];
         }
         nodes[json["nodes"][node]["id"]] = new this.createNode(
               json["nodes"][node]["id"],
               json["nodes"][node]["name"],
               json["nodes"][node]["java_class"],
               json["nodes"][node]["x"],
               json["nodes"][node]["y"],
               json["nodes"][node]["color"],
               top_id,
               button_id,
               null
            )
         buttons[button_id] = nodes[json["nodes"][node]["id"]];
         tops[top_id] = nodes[json["nodes"][node]["id"]];
      }
      var edges = json["edges"];
      for(node in nodes){
         nodes[node].addNodes(tops,edges);
      }
      return nodes;
   },

   getNodesFromDB: function (plan) {
      var nodes = {};
      var tops = {};
      var buttons = {};
      var gid = 1;
      for(operator in plan["operators"]){
         var node;
         var name = plan["operators"][operator]["name"];
         var java_class = plan["operators"][operator]["java_class"];
         var x = plan["operators"][operator]["x"];
         var y = plan["operators"][operator]["y"];
         var parameters = plan["operators"][operator]["parameters"];
         var color = plan["operators"][operator]["color"];
         var id = gid++;
         node = new this.createNode(id, name, java_class, x, y, color, null, null, parameters)
         for(connectors in plan["operators"][operator]["connects_to"]){
            var via;
            for(type in plan["operators"][operator]["connects_to"][connectors][0]){
               if(type == "via"){
                  via = plan["operators"][operator]["connects_to"][connectors][0][type];
               }else{
                  node.nextNodes.push(type);
               }
            }
            node.nextNodesEdges.push(via);
         }
         nodes[name] = node;
      }
      for(node in nodes){
         var nextNames = nodes[node].nextNodes;
         if(nextNames.length < 1) continue;
         buttons[nodes[node].name] = buttons[nodes[node].name] || gid++;
         nodes[nodes[node].name].button = buttons[nodes[node].name];
         nodes[node].nextNodes = [];
         for(name in nextNames){
            tops[nodes[nextNames[name]].name] = tops[nodes[nextNames[name]].name] || gid++;
            nodes[nodes[nextNames[name]].name].top = tops[nodes[nextNames[name]].name];
            nodes[node].nextNodes.push(nodes[nextNames[name]]);
         }
      }
      var fullNodes = {};
      fullNodes["nodes"] = nodes;
      fullNodes["tops"] = tops;
      fullNodes["buttons"] = buttons;
      return fullNodes;
   },

   getViewFromExec: function (execPlan, xstep, ystep, color){
      const COLORS = color
      const XStep = xstep, YStep = ystep;
      var plan = {};
      plan["operators"] = [];
      for(operators in execPlan["stages"]){
         plan["operators"] = plan["operators"].concat(execPlan["stages"][operators]["operators"]);
      }
      var fullNodes = this.getNodesFromDB(plan);
      var sorted = {};
      var visit = function (parentName, node, sorted, level) {
         for (var i = 0; i < node.nextNodes.length; i++) {
            visit(node.name,node.nextNodes[i], sorted, level+1);
         }
         sorted[node.name] = sorted[node.name] || {};
         sorted[node.name]["level"] = sorted[node.name]["level"] || -1;
         if(sorted[node.name]["level"] < level){
            sorted[node.name]["level"] = level;
            sorted[node.name]["parent"] = parentName;
         }
      }
      for(name in fullNodes["nodes"]){
         visit(null,fullNodes["nodes"][name], sorted, 0);
      }
      var execGraph = {nodes:{}, edges: []}
      var node;
      var levels = [], levelCount = [];
      for(name in sorted){
         levelCount[sorted[name]["level"]] = levelCount[sorted[name]["level"]] + 1 || 1;
      }
      for (var i = 0; i < execPlan["stages"].length; i++) {
         var color = COLORS[execPlan["stages"][i].platform];
         for (var j = 0; j < execPlan["stages"][i]["operators"].length; j++) {
            node = execPlan["stages"][i]["operators"][j];
            execGraph.nodes[node.name] = {};
            execGraph.nodes[node.name]["color"] = color;

            execGraph.nodes[node.name]["y"] = sorted[node.name]["level"] * YStep;
            levels[sorted[node.name]["level"]] = levels[sorted[node.name]["level"]] + 1 || 0;

            if(levelCount[sorted[node.name]["level"]] < 2)
               execGraph.nodes[node.name]["x"] = XStep / 2;
            else{
               execGraph.nodes[node.name]["x"] = levels[sorted[node.name]["level"]] * XStep;
            }

            for(next in fullNodes["nodes"][node.name].nextNodes){
               execGraph.edges.push({source: node.name,
                  target: fullNodes["nodes"][node.name].nextNodes[next].name,
                  type: fullNodes["nodes"][node.name].nextNodesEdges[next]
               })
               fullNodes["nodes"][node.name].nextNodes[next]
            }
         }
      }

      for(name in execGraph.nodes){
         if(sorted[name].parent && fullNodes.nodes[sorted[name].parent].nextNodes.length == 1){
            execGraph.nodes[name].x = execGraph.nodes[sorted[name].parent].x;
         }
      }

      return execGraph;
   }
}

module.exports = Graph;
