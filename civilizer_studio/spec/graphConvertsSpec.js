var Graph = require("../helpers/graph")
var graph = new Graph();

describe("Graph convert - (getNodeFromView) -", function() {
  describe("From Json to Nodes -", function(){

    it("Empty Graph", function() {
      expect(graph.getNodeFromView("{}")).toEqual({});
    });

    it("one node - no edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = 11;
      var button = 22;
      var parameters = null;

      var json = '{ "nodes": [ { "name": "'+name+'", "id": '+id+', "x": '+x+', "y": '+y+', "color": "'+color+'", "java_class": "'+java_class+'", "connectors": [ { "id": '+button+', "type": "bottomConnector" }, { "id": '+top+', "type": "topConnector" } ] } ], "edges": [] }';
      var expNode = graph.createNode(id, name, java_class, x, y, color, top, button, parameters);
      var nodes = graph.getNodeFromView(JSON.parse(json))

      expect(Object.keys(nodes).length).toBe(1);

      var node = nodes[id];

      expect(node.id).toEqual(expNode.id);
      expect(node.x).toEqual(expNode.x);
      expect(node.y).toEqual(expNode.y);
      expect(node.name).toEqual(expNode.name);
      expect(node.color).toEqual(expNode.color);
      expect(node.java_class).toEqual(expNode.java_class);
      expect(node.top).toEqual(expNode.top);
      expect(node.button).toEqual(expNode.button);
      expect(node.parameters).toEqual(expNode.parameters);
    });

    it("Three node - two edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = null;
      var button = 20;
      var parameters = null;

      var id2 = 2;
      var y2 = 30;
      var x2 = 40;
      var color2 = "#FF00FF";
      var java_class2 = "java.lang.int";
      var name2 = "op2";
      var top2 = 21;
      var button2 = 22;
      var parameters2 = null;

      var id3 = 3;
      var y3 = 50;
      var x3 = 60;
      var color3 = "#FF00FF";
      var java_class3 = "java.lang.long";
      var name3 = "op3";
      var top3 = 23;
      var button3 = null;
      var parameters3 = null;

      var json = '{ "nodes": [ { "name": "'+name+'", "id": '+id+', "x": '+x+', "y": '+y+', "color": "'+color+'", "java_class": "'+java_class+'", "connectors": [ { "id": '+button+', "type": "bottomConnector" } ] },\
                               { "name": "'+name2+'", "id": '+id2+', "x": '+x2+', "y": '+y2+', "color": "'+color2+'", "java_class": "'+java_class2+'", "connectors": [ { "id": '+top2+', "type": "topConnector" }, {"id": '+button2+', "type": "bottomConnector" } ] },\
                               { "name": "'+name3+'", "id": '+id3+', "x": '+x3+', "y": '+y3+', "color": "'+color3+'", "java_class": "'+java_class3+'", "connectors": [ { "id": '+top3+', "type": "topConnector" } ] } ],\
                    "edges": [{ "source": '+button+', "destination": '+top2+' }, { "source": '+button2+', "destination": '+top3+' } ] }';
      var expNode = graph.createNode(id, name, java_class, x, y, color, top, button, parameters);
      var expNode2 = graph.createNode(id2, name2, java_class2, x2, y2, color2, top2, button2, parameters2);
      var expNode3 = graph.createNode(id3, name3, java_class3, x3, y3, color3, top3, button3, parameters3);
      // expNode.nextNodes = [expNode2];
      // expNode2.nextNodes = [expNode3];
      var nodes = graph.getNodeFromView(JSON.parse(json))

      expect(Object.keys(nodes).length).toBe(3);

      expect(nodes[id].nextNodes[0].id).toEqual(expNode2.id);
      expect(nodes[id].nextNodes[0].x).toEqual(expNode2.x);
      expect(nodes[id].nextNodes[0].y).toEqual(expNode2.y);
      expect(nodes[id].nextNodes[0].name).toEqual(expNode2.name);
      expect(nodes[id].nextNodes[0].color).toEqual(expNode2.color);
      expect(nodes[id].nextNodes[0].java_class).toEqual(expNode2.java_class);
      expect(nodes[id].nextNodes[0].top).toEqual(expNode2.top);
      expect(nodes[id].nextNodes[0].button).toEqual(expNode2.button);
      expect(nodes[id].nextNodes[0].parameters).toEqual(expNode2.parameters);

      expect(nodes[id2].nextNodes[0].id).toEqual(expNode3.id);
      expect(nodes[id2].nextNodes[0].x).toEqual(expNode3.x);
      expect(nodes[id2].nextNodes[0].y).toEqual(expNode3.y);
      expect(nodes[id2].nextNodes[0].name).toEqual(expNode3.name);
      expect(nodes[id2].nextNodes[0].color).toEqual(expNode3.color);
      expect(nodes[id2].nextNodes[0].java_class).toEqual(expNode3.java_class);
      expect(nodes[id2].nextNodes[0].top).toEqual(expNode3.top);
      expect(nodes[id2].nextNodes[0].button).toEqual(expNode3.button);
      expect(nodes[id2].nextNodes[0].parameters).toEqual(expNode3.parameters);
    });
  })
});

describe("Graph convert - (getPlanFromNodes) -", function() {
  describe("From Nodes to Plan -", function(){
    it("Empty Plan", function() {
      expect(graph.getPlanFromNodes([])).toEqual({"operators":[], "sink_operators": []});
    });

    it("one node - no edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = 11;
      var button = null;
      var parameters = null;

      var expNode = graph.createNode(id, name, java_class, x, y, color, top, button, parameters);
      var plan = graph.getPlanFromNodes([expNode]);

      expect(plan["sink_operators"].length).toBe(1);
      expect(plan["operators"].length).toBe(1);

      expect(plan["sink_operators"][0]).toEqual(name);
      expect(plan["operators"][0]["name"]).toEqual(name);
      expect(plan["operators"][0]["x"]).toEqual(x);
      expect(plan["operators"][0]["y"]).toEqual(y);
      expect(plan["operators"][0]["color"]).toEqual(color);
      expect(plan["operators"][0]["java_class"]).toEqual(java_class);
      expect(plan["operators"][0]["parameters"]).toEqual(parameters);
      expect(plan["operators"][0]["connects_to"]).toEqual({});
    });

    it("Three node - two edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = null;
      var button = 20;
      var parameters = {"name": "url", "type": "java.lang.String"};

      var id2 = 2;
      var y2 = 30;
      var x2 = 40;
      var color2 = "#FF00FF";
      var java_class2 = "java.lang.int";
      var name2 = "op2";
      var top2 = 21;
      var button2 = 22;
      var parameters2 = {"name": "typeClass", "type": "java.util.Collection"};

      var id3 = 3;
      var y3 = 50;
      var x3 = 60;
      var color3 = "#FF00FF";
      var java_class3 = "java.lang.long";
      var name3 = "op3";
      var top3 = 23;
      var button3 = null;
      var parameters3 = {"name": "input", "type": "stdout"};

      var expNode = graph.createNode(id, name, java_class, x, y, color, top, button, parameters);
      var expNode2 = graph.createNode(id2, name2, java_class2, x2, y2, color2, top2, button2, parameters2);
      var expNode3 = graph.createNode(id3, name3, java_class3, x3, y3, color3, top3, button3, parameters3);

      expNode.nextNodes = [expNode2];
      expNode2.nextNodes = [expNode3];

      var nodes = [];
      nodes.push(expNode);
      nodes.push(expNode2);
      nodes.push(expNode3);

      var plan = graph.getPlanFromNodes(nodes);

      expect(plan["sink_operators"].length).toBe(1);
      expect(plan["operators"].length).toBe(3);

      expect(plan["sink_operators"][0]).toEqual(name3);

      expect(plan["operators"][0]["name"]).toEqual(name);
      expect(plan["operators"][0]["x"]).toEqual(x);
      expect(plan["operators"][0]["y"]).toEqual(y);
      expect(plan["operators"][0]["color"]).toEqual(color);
      expect(plan["operators"][0]["java_class"]).toEqual(java_class);
      expect(plan["operators"][0]["parameters"]).toEqual(parameters);
      expect(Object.keys(plan["operators"][0]["connects_to"]["0"][0])).toEqual([name2]);

      expect(plan["operators"][1]["name"]).toEqual(name2);
      expect(plan["operators"][1]["x"]).toEqual(x2);
      expect(plan["operators"][1]["y"]).toEqual(y2);
      expect(plan["operators"][1]["color"]).toEqual(color2);
      expect(plan["operators"][1]["java_class"]).toEqual(java_class2);
      expect(plan["operators"][1]["parameters"]).toEqual(parameters2);
      expect(Object.keys(plan["operators"][1]["connects_to"]["0"][0])).toEqual([name3]);

      expect(plan["operators"][2]["name"]).toEqual(name3);
      expect(plan["operators"][2]["x"]).toEqual(x3);
      expect(plan["operators"][2]["y"]).toEqual(y3);
      expect(plan["operators"][2]["color"]).toEqual(color3);
      expect(plan["operators"][2]["java_class"]).toEqual(java_class3);
      expect(plan["operators"][2]["parameters"]).toEqual(parameters3);
      expect(plan["operators"][2]["connects_to"]).toEqual({});
    });
  })
});


describe("Graph convert - (getNodesFromDB) -", function() {
  describe("From Plan to Nodes -", function(){
    it("Empty Nodes", function() {
      expect(graph.getNodesFromDB({})).toEqual({"nodes": {}, "tops": {}, "buttons": {}});
    });

    it("one operator - no edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = 11;
      var button = null;
      var parameters = null;

      var plan = {
        "sink_operators": [name],
        "operators":[
          {
            "name": name,
            "x": x,
            "y": y,
            "color": color,
            "java_class": java_class,
            "parameters": parameters,
            "connects_to": {},
          }
        ]
      };

      var fullNodes = graph.getNodesFromDB(plan);

      expect(Object.keys(fullNodes["nodes"]).length).toBe(1);

      expect(fullNodes["nodes"][name]["name"]).toEqual(name);
      expect(fullNodes["nodes"][name]["x"]).toEqual(x);
      expect(fullNodes["nodes"][name]["y"]).toEqual(y);
      expect(fullNodes["nodes"][name]["color"]).toEqual(color);
      expect(fullNodes["nodes"][name]["java_class"]).toEqual(java_class);
      expect(fullNodes["nodes"][name]["parameters"]).toEqual(parameters);
      expect(fullNodes["nodes"][name]["nextNodes"]).toEqual([]);
    });

    it("Three node - two edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = null;
      var button = 20;
      var parameters = {"name": "url", "type": "java.lang.String"};

      var id2 = 2;
      var y2 = 30;
      var x2 = 40;
      var color2 = "#FF00FF";
      var java_class2 = "java.lang.int";
      var name2 = "op2";
      var top2 = 21;
      var button2 = 22;
      var parameters2 = {"name": "typeClass", "type": "java.util.Collection"};

      var id3 = 3;
      var y3 = 50;
      var x3 = 60;
      var color3 = "#FF00FF";
      var java_class3 = "java.lang.long";
      var name3 = "op3";
      var top3 = 23;
      var button3 = null;
      var parameters3 = {"name": "input", "type": "stdout"};

      var plan = {
        "sink_operators": [name3],
        "operators":[
          {
            "name": name,
            "x": x,
            "y": y,
            "color": color,
            "java_class": java_class,
            "parameters": parameters,
            "connects_to": {
              "0": [
                {[name2]: "0"}
              ]
            },
          },
          {
            "name": name2,
            "x": x2,
            "y": y2,
            "color": color2,
            "java_class": java_class2,
            "parameters": parameters2,
            "connects_to": {
              "0": [
                {[name3]: "0"}
              ]
            },
          },
          {
            "name": name3,
            "x": x3,
            "y": y3,
            "color": color3,
            "java_class": java_class3,
            "parameters": parameters3,
            "connects_to": {},
          }
        ]
      };

      var fullNodes = graph.getNodesFromDB(plan);

      expect(Object.keys(fullNodes["nodes"]).length).toBe(3);
      expect(Object.keys(fullNodes["tops"]).length).toBe(2);
      expect(Object.keys(fullNodes["buttons"]).length).toBe(2);

      expect(fullNodes["nodes"][name]["name"]).toEqual(name);
      expect(fullNodes["nodes"][name]["x"]).toEqual(x);
      expect(fullNodes["nodes"][name]["y"]).toEqual(y);
      expect(fullNodes["nodes"][name]["color"]).toEqual(color);
      expect(fullNodes["nodes"][name]["java_class"]).toEqual(java_class);
      expect(fullNodes["nodes"][name]["parameters"]).toEqual(parameters);
      expect(fullNodes["nodes"][name]["nextNodes"].length).toBe(1);



      expect(fullNodes["nodes"][name2]["name"]).toEqual(name2);
      expect(fullNodes["nodes"][name2]["x"]).toEqual(x2);
      expect(fullNodes["nodes"][name2]["y"]).toEqual(y2);
      expect(fullNodes["nodes"][name2]["color"]).toEqual(color2);
      expect(fullNodes["nodes"][name2]["java_class"]).toEqual(java_class2);
      expect(fullNodes["nodes"][name2]["parameters"]).toEqual(parameters2);
      expect(fullNodes["nodes"][name2]["nextNodes"].length).toBe(1);

      expect(fullNodes["nodes"][name].nextNodes[0]["name"]).toEqual(name2);
      expect(fullNodes["nodes"][name].nextNodes[0]["x"]).toEqual(x2);
      expect(fullNodes["nodes"][name].nextNodes[0]["y"]).toEqual(y2);
      expect(fullNodes["nodes"][name].nextNodes[0]["color"]).toEqual(color2);
      expect(fullNodes["nodes"][name].nextNodes[0]["java_class"]).toEqual(java_class2);
      expect(fullNodes["nodes"][name].nextNodes[0]["parameters"]).toEqual(parameters2);
      expect(fullNodes["nodes"][name].nextNodes[0]["nextNodes"].length).toBe(1);



      expect(fullNodes["nodes"][name3]["name"]).toEqual(name3);
      expect(fullNodes["nodes"][name3]["x"]).toEqual(x3);
      expect(fullNodes["nodes"][name3]["y"]).toEqual(y3);
      expect(fullNodes["nodes"][name3]["color"]).toEqual(color3);
      expect(fullNodes["nodes"][name3]["java_class"]).toEqual(java_class3);
      expect(fullNodes["nodes"][name3]["parameters"]).toEqual(parameters3);
      expect(fullNodes["nodes"][name3]["nextNodes"].length).toBe(0);

      expect(fullNodes["nodes"][name2].nextNodes[0]["name"]).toEqual(name3);
      expect(fullNodes["nodes"][name2].nextNodes[0]["x"]).toEqual(x3);
      expect(fullNodes["nodes"][name2].nextNodes[0]["y"]).toEqual(y3);
      expect(fullNodes["nodes"][name2].nextNodes[0]["color"]).toEqual(color3);
      expect(fullNodes["nodes"][name2].nextNodes[0]["java_class"]).toEqual(java_class3);
      expect(fullNodes["nodes"][name2].nextNodes[0]["parameters"]).toEqual(parameters3);
      expect(fullNodes["nodes"][name2].nextNodes[0]["nextNodes"].length).toBe(0);

    });
  })
});


describe("Graph convert - (getViewedJson) -", function() {
  describe("From Full Nodes to Json -", function(){
    it("Empty Plan", function() {
      expect(graph.getViewedJson({"nodes": {}, "tops": {}, "buttons": {}})).toEqual({"nodes": [], "edges": []});
    });

    it("one node - no edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = null;
      var button = null;
      var parameters = null;

      var expNode = graph.createNode(id, name, java_class, x, y, color, top, button, parameters);
      var json = graph.getViewedJson({"nodes": {[name]: expNode, }, "tops": {}, "buttons": {}});

      expect(json["nodes"].length).toBe(1);
      expect(json["edges"].length).toBe(0);

      expect(json["nodes"][0]["name"]).toEqual(name);
      expect(json["nodes"][0]["x"]).toEqual(x);
      expect(json["nodes"][0]["y"]).toEqual(y);
      expect(json["nodes"][0]["color"]).toEqual(color);
      expect(json["nodes"][0]["java_class"]).toEqual(java_class);
      expect(json["nodes"][0]["parameters"]).toEqual(parameters);
      expect(json["nodes"][0]["connectors"]).toBeUndefined();
    });

    it("Three node - two edges", function() {
      var id = 1;
      var y = 10;
      var x = 20;
      var color = "#FFFFFF";
      var java_class = "java.lang.String";
      var name = "op1";
      var top = null;
      var button = 20;
      var parameters = {"name": "url", "type": "java.lang.String"};

      var id2 = 2;
      var y2 = 30;
      var x2 = 40;
      var color2 = "#FF00FF";
      var java_class2 = "java.lang.int";
      var name2 = "op2";
      var top2 = 21;
      var button2 = 22;
      var parameters2 = {"name": "typeClass", "type": "java.util.Collection"};

      var id3 = 3;
      var y3 = 50;
      var x3 = 60;
      var color3 = "#FF00FF";
      var java_class3 = "java.lang.long";
      var name3 = "op3";
      var top3 = 23;
      var button3 = null;
      var parameters3 = {"name": "input", "type": "stdout"};

      var expNode = graph.createNode(id, name, java_class, x, y, color, top, button, parameters);
      var expNode2 = graph.createNode(id2, name2, java_class2, x2, y2, color2, top2, button2, parameters2);
      var expNode3 = graph.createNode(id3, name3, java_class3, x3, y3, color3, top3, button3, parameters3);

      expNode.nextNodes = [expNode2];
      expNode2.nextNodes = [expNode3];

      var json = graph.getViewedJson({"nodes": {[name]: expNode, [name2]: expNode2, [name3]: expNode3}, "tops": {[name2]: expNode2, [name3]: expNode3}, "buttons": {[name]: expNode, [name2]: expNode2}});

      expect(json["nodes"].length).toBe(3);
      expect(json["edges"].length).toBe(2);

      expect(json["nodes"][0]["name"]).toEqual(name);
      expect(json["nodes"][0]["x"]).toEqual(x);
      expect(json["nodes"][0]["y"]).toEqual(y);
      expect(json["nodes"][0]["color"]).toEqual(color);
      expect(json["nodes"][0]["java_class"]).toEqual(java_class);
      expect(json["nodes"][0]["parameters"]).toEqual(parameters);
      expect(json["nodes"][0]["connectors"][0]["type"]).toEqual("bottomConnector");

      expect(json["nodes"][1]["name"]).toEqual(name2);
      expect(json["nodes"][1]["x"]).toEqual(x2);
      expect(json["nodes"][1]["y"]).toEqual(y2);
      expect(json["nodes"][1]["color"]).toEqual(color2);
      expect(json["nodes"][1]["java_class"]).toEqual(java_class2);
      expect(json["nodes"][1]["parameters"]).toEqual(parameters2);
      expect(json["nodes"][1]["connectors"][0]["type"]).toEqual("topConnector");
      expect(json["nodes"][1]["connectors"][1]["type"]).toEqual("bottomConnector");

      expect(json["nodes"][2]["name"]).toEqual(name3);
      expect(json["nodes"][2]["x"]).toEqual(x3);
      expect(json["nodes"][2]["y"]).toEqual(y3);
      expect(json["nodes"][2]["color"]).toEqual(color3);
      expect(json["nodes"][2]["java_class"]).toEqual(java_class3);
      expect(json["nodes"][2]["parameters"]).toEqual(parameters3);
      expect(json["nodes"][2]["connectors"][0]["type"]).toEqual("topConnector");

    });
  })
});


describe("Graph convert - (Integration) -", function() {
  describe("From Json to Json -", function(){
    it("full Path", function() {
      var json = JSON.parse('{\
"nodes": [\
{\
"name": "FileText",\
"id": 1,\
"x": 125,\
"y": 31,\
"color": "#F15B29",\
"connectors": [\
{\
"id": 8,\
"type": "bottomConnector"\
}\
]\
},\
{\
"name": "SparkObjectFile",\
"id": 2,\
"x": 337,\
"y": 28,\
"color": "#F15B29",\
"connectors": [\
{\
"id": 9,\
"type": "bottomConnector"\
}\
]\
},\
{\
"name": "Sort",\
"id": 3,\
"x": 240,\
"y": 165,\
"color": "#F15B30",\
"connectors": [\
{\
"id": 10,\
"type": "bottomConnector"\
},\
{\
"id": 11,\
"type": "topConnector"\
}\
]\
},\
{\
"name": "Map",\
"id": 4,\
"x": 80,\
"y": 271,\
"color": "#F15B30",\
"connectors": [\
{\
"id": 12,\
"type": "bottomConnector"\
},\
{\
"id": 13,\
"type": "topConnector"\
}\
]\
},\
{\
"name": "Reduce",\
"id": 5,\
"x": 369,\
"y": 263,\
"color": "#F15B30",\
"connectors": [\
{\
"id": 14,\
"type": "bottomConnector"\
},\
{\
"id": 15,\
"type": "topConnector"\
}\
]\
},\
{\
"name": "Count",\
"id": 6,\
"x": 228,\
"y": 368,\
"color": "#F15B30",\
"connectors": [\
{\
"id": 16,\
"type": "bottomConnector"\
},\
{\
"id": 17,\
"type": "topConnector"\
}\
]\
},\
{\
"name": "LocalCallback",\
"id": 7,\
"x": 200,\
"y": 550,\
"color": "#F15B26",\
"connectors": [\
{\
"id": 18,\
"type": "topConnector"\
}\
]\
}\
],\
"edges": [\
{\
"source": 8,\
"destination": 11\
},\
{\
"source": 9,\
"destination": 11\
},\
{\
"source": 10,\
"destination": 13\
},\
{\
"source": 10,\
"destination": 15\
},\
{\
"source": 14,\
"destination": 17\
},\
{\
"source": 12,\
"destination": 17\
},\
{\
"source": 16,\
"destination": 18\
}\
]\
}');
      var nodes = graph.getNodeFromView(json);
      var plan = graph.getPlanFromNodes(nodes);
      var fullNodes = graph.getNodesFromDB(plan);
      var final_json = graph.getViewedJson(fullNodes);
      // expect(final_json).toEqual(json);
      expect(final_json["nodes"].length).toBe(json["nodes"].length);
      expect(final_json["edges"].length).toBe(json["edges"].length);
    });
  });
});