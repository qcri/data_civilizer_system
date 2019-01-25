'use strict';

appControllers.controller('editorController', ['$scope', 'prompt', 'Modelfactory', 'flowchartConstants', 'RheemAPI', 'excutionPlan', '$http', 'plansConversions', '$timeout', '$uibModal', function($scope, prompt, Modelfactory, flowchartConstants, RheemAPI, excutionPlan, $http, plansConversions, $timeout, $uibModal) {

  $scope.useGEM = false;
  $scope.simulate = false;

  // Drag Area Collapse Blocks
  $scope.isCollapsed = false;
  $scope.arrowDirection = true;

  $scope.paramIndex  = -1;

  $scope.soucresCollapseClicked = function() {
    $scope.sourcesIsCollapsed = !$scope.sourcesIsCollapsed;
  };
  $scope.unaryOperatorsCollapseClicked = function() {
    $scope.unaryOperatorsIsCollapsed = !$scope.unaryOperatorsIsCollapsed;
  };
  $scope.binaryOperatorsCollapseClicked = function() {
    $scope.binaryOperatorsIsCollapsed = !$scope.binaryOperatorsIsCollapsed;
  };
  $scope.trinaryOperatorsCollapseClicked = function() {
    $scope.trinaryOperatorsIsCollapsed = !$scope.trinaryOperatorsIsCollapsed;
  };
  $scope.sinkCollapseClicked = function() {
    $scope.sinkIsCollapsed = !$scope.sinkIsCollapsed;
  };
  $scope.confCollapseClicked = function() {
    $scope.confIsCollapsed = !$scope.confIsCollapsed;
  };

  var operatorIndex = {};
  $scope.unaryOperators = [];
  $scope.binaryOperators = [];
  $scope.trinaryOperators = [];
  $scope.datasources = [];
  $scope.sinks = [];

  var bindOperatorsData = function(operators) {
    for(var operator of operators) {
      operatorIndex[operator.class] = operator;
      if(operator.loop) {
        $scope.trinaryOperators.push(operator);
      } else if(operator.nb_inputs && operator.nb_inputs == 1 && operator.nb_outputs) {
        $scope.unaryOperators.push(operator);
      } else if(operator.nb_inputs && operator.nb_inputs == 2 && operator.nb_outputs) {
// When in simulation mode, enable advanced progress on gather...
if($scope.simulate) operator.features.progress = true;
        $scope.binaryOperators.push(operator);
      } else if(operator.nb_inputs && operator.nb_inputs > 2 && operator.nb_outputs) {
        $scope.trinaryOperators.push(operator);
      } else if(operator.nb_inputs && !operator.nb_outputs) {
        $scope.sinks.push(operator);
      } else if(operator.nb_outputs && !operator.nb_inputs) {
// When in simulation mode, force source operators to serializer...
if($scope.simulate) operator.features.parallel = false;
        $scope.datasources.push(operator);
      }
    }
  };

  // GET OPERATORS FROM API
  // RheemAPI.getBasicOperators({}, function(data) {
  //   bindOperatorsData(data.operators);
  // });
  //
  // RheemAPI.getJavaOperators({}, function(data) {
  //   bindOperatorsData(data.operators);
  // });
  //
  // RheemAPI.getSparkOperators({}, function(data) {
  //   bindOperatorsData(data.operators);
  // });

  RheemAPI.getRheemOperator({}, function(data) {
    bindOperatorsData(data.operators);
  });

/*
// Not used - MDH 2019/01/16
  // Configurations
  $scope.configurations = [
    {
      name: "Conf #1"
    },
    {
      name: "Conf #2"
    },
    {
      name: "Conf #3"
    },
    {
      name: "Conf #4"
    },
  ];
*/

  $scope.itemSelected = function(item) {
  };

  // Drag & Drop
  var deleteKeyCode = 46;
  var ctrlKeyCode = 17;
  var aKeyCode = 65;
  var escKeyCode = 27;
//345678901234567890123456789012345678901234567890123456789012345678901234567890
  //
  // Node and connector IDs are used by the ngFlowchart module to identify
  // chart elements and must be unique within the currently displayed chart.
  // However, they are not stored with the model, but are reassigned each
  // time a saved plan is loaded.
  //
  var maxNodeID = 0; // was 10
  var minConnectorID = 0; // was 20
  var nextParamID = 30;
  $scope.ctrlDown = false;
  var dragOptions = {
    placeholder: true
  };

  var model = {
    nodes: [],
    edges: []
  };

  $scope.flowchartselected = [];
  var modelservice = Modelfactory(model, $scope.flowchartselected);

  $scope.model = model;
  $scope.modelservice = modelservice;

  $scope.keyDown = function(evt) {
    if(evt.keyCode === ctrlKeyCode) {
      $scope.ctrlDown = true;
      evt.stopPropagation();
      evt.preventDefault();
    }
  };

  $scope.keyUp = function(evt) {
    if(evt.keyCode === deleteKeyCode) {
      modelservice.deleteSelected();
    }

    if(evt.keyCode == aKeyCode && $scope.ctrlDown) {
      modelservice.selectAll();
    }

    if(evt.keyCode == escKeyCode) {
      modelservice.deselectAll();
    }

    if(evt.keyCode === ctrlKeyCode) {
      $scope.ctrlDown = false;
      evt.stopPropagation();
      evt.preventDefault();
    }
  };

  var enterNodeName = function(nodename) {
    // Gather current node names to ensure new node name is unique
    var nodeNames = model.nodes.map(function(node) { return node.name; });
    do {
      var nodeName = prompt("Enter a node name:", nodename);
      console.log(nodeName);
      if(!nodeName) {
        break;
      } else {
        if(nodeNames.indexOf(nodeName) != -1) {
          alert("Node name already exist");
          nodeName = "";
        }
      }
    } while(!nodeName);
    return nodeName;
  };

  $scope.addNewNode = function(nodetype, node, nodeClass) {
    var nodeName = enterNodeName(node.class.split(".").pop());
    if(!nodeName) {
      return;
    }

    var getConnectorsByType = function(nodetype, node) {
      var connectorsArr = [];
      function addConnector(type, broadcast) {
        var connector = {
          "id": --minConnectorID,
          "type": type
        };
        if(broadcast) {
          connector["broadcast"] = true;
        }
        connectorsArr.push(connector);
      }
      switch(nodetype) {
        case "sink":
          addConnector(flowchartConstants.topConnectorType);
          break;
        case "datasource":
          addConnector(flowchartConstants.bottomConnectorType);
          break;
        case "operator":
          for(var i = 0; (i < node.nb_inputs) && (i < 3); i++) {
            addConnector(flowchartConstants.topConnectorType);
          }
          for(var i = 0; (i < node.nb_outputs) && (i < 3); i++) {
            addConnector(flowchartConstants.bottomConnectorType);
          }
//        if(node.supportBroadcast){
          if(node.features && node.features.broadcast) {
            addConnector(flowchartConstants.topConnectorType, true);
            addConnector(flowchartConstants.topConnectorType, true);
          }
          break;
      }
      return connectorsArr;
    }

    var newNode = createNewNode(nodetype, node, nodeClass, nodeName);
    newNode.id = ++maxNodeID,
    newNode.connectors = getConnectorsByType(nodetype, node);
    model.nodes.push(newNode);
  };

  var createNewNode = function(nodetype,  node, nodeClass, nodeName) {
    node = JSON.parse(JSON.stringify(node));
    console.log(node);

    var getNodeParams = function(parameters) {
      for(var item in parameters) {
        for(var paramsItem in parameters[item]) {
          if(parameters[item][paramsItem].inputType == 'UDF') {
            var udfTmpl = parameters[item][paramsItem].udfTmpl;
            parameters[item][paramsItem].value = udfTmpl.replace(/OPNAME/g, nodeName)
                                                        .replace(/PARAMNAME/g, parameters[item][paramsItem].name);
          } else {
            parameters[item][paramsItem].value = "";
          }
          parameters[item][paramsItem].id = nextParamID++;
        }
      }
      return parameters;
    }

    var getColorCodeByType = function(nodetype){
      switch(nodetype) {
        case "sink":       return '#F15B26';
        case "datasource": return '#F15B29';
        case "operator":   return '#F15B30';
      }
    }

    var newNode = {
      name: nodeName,
//    id: ++maxNodeID,
      x: 10,
      y: 10,
      color: getColorCodeByType(nodetype),
//    connectors: getConnectorsByType(nodetype, node),
      java_class: node.class,
      parameters: getNodeParams(node.parameters),
      interactive: node.interactive,
      type: nodeClass,
      selectedConstructor: -1,
//    isBroadCast: node.supportBroadcast,
      isBroadCast: node.features && node.features.broadcast,
      features: node.features,
      baseurl: node.baseurl
    };

    return newNode;
  };

/*
  // Note currently used - MDH 2019/01/14
  $scope.activateWorkflow = function() {
    angular.forEach($scope.model.edges, function(edge) {
      edge.active = !edge.active;
    });
  };

  $scope.addNewInputConnector = function() {
    var connectorName = prompt("Enter a connector name:", "New connector");
    if(!connectorName) {
      return;
    }

    var selectedNodes = modelservice.nodes.getSelectedNodes($scope.model);
    for(var i = 0; i < selectedNodes.length; ++i) {
      var node = selectedNodes[i];
      node.connectors.push({id: --minConnectorID, type: flowchartConstants.topConnectorType});
    }
  };

  $scope.addNewOutputConnector = function() {
    var connectorName = prompt("Enter a connector name:", "New connector");
    if(!connectorName) {
      return;
    }

    var selectedNodes = modelservice.nodes.getSelectedNodes($scope.model);
    for(var i = 0; i < selectedNodes.length; ++i) {
      var node = selectedNodes[i];
      node.connectors.push({id: --minConnectorID, type: flowchartConstants.bottomConnectorType});
    }
  };
*/

  $scope.deleteSelected = function() {
    modelservice.deleteSelected();
  };

  $scope.callbacks = {
    edgeDoubleClick: function() {
      //console.log('Edge double clicked.');
    },
    edgeMouseOver: function() {
      //console.log('mouserover');
    },
    isValidEdge: function(source, destination) {
      return source.type === flowchartConstants.bottomConnectorType && destination.type === flowchartConstants.topConnectorType;
    },
    nodeCallbacks: {
      'singleClick': function(item) {
        alert("hello");
      },
      'doubleClick': function(item) {
        $scope.item = item;
        $scope.selectedNodeData = item;
        $scope.selectedParam = [];
        $scope.selectedNodeParams = [];
        for(var x in $scope.selectedNodeData.parameters) {
          var paramStr = "(";
          for(var y in $scope.selectedNodeData.parameters[x]) {
            paramStr += $scope.selectedNodeData.parameters[x][y].type + ",\n";
          }
          paramStr = paramStr.trim().slice(0, -1);
          paramStr += ")";
          $scope.selectedNodeParams.push(paramStr);
        }
        if(item.selectedConstructor != -1) {
          // $scope.selectedParam = $scope.selectedNodeData.parameters[item.selectedConstructor];
          $scope.paramIndex = item.selectedConstructor;
          $scope.selectedParam = $scope.model.nodes[$scope.model.nodes.indexOf($scope.selectedNodeData)].parameters[item.selectedConstructor];
        } else {
          $scope.paramIndex = -1;
          $scope.selectedParam = [];
        }
        $scope.showParamPanel = true;

        // Automatically selected the first parameter set to display it
        $scope.paramClicked($scope.paramIndex != -1 ? $scope.paramIndex : 0);
      }
    }
  };

  // Plans
  $scope.currentPlan = -1;

  RheemAPI.getPlans({}, function(data) {
    $scope.plans = data;
  });

  $scope.createNewPlan = function() {
    if($scope.ctrlDown) {
      deleteSelectedPlan();
    } else {
      window.location.reload();
    }
  };

  var deleteSelectedPlan = function() {
    if($scope.currentPlan != -1) {
      $scope.ctrlDown = false;
      var name = getSelectedPlan($scope.currentPlan).name;
      if(confirm("Delete plan '" + name + "'?")) {
        RheemAPI.deletePlan({id: $scope.currentPlan}, function(data) {
          alert("Plan '" + name + "' deleted.");
          $scope.currentPlan = -1;
          RheemAPI.getPlans({}, function(data) {
            $scope.plans = data;
          });
        });
      }
    }
  }

  $scope.showCurrentPlan = function() {
    console.log($scope.currentPlan);
    $scope.plan = getSelectedPlan($scope.currentPlan);
    console.log($scope.plan);
    $scope.paramIndex = -1;
    $scope.selectedParam = [];
    $scope.selectedNodeParams = [];
    $scope.showParamPanel = false;
    var nodes = plansConversions.getNodesFromDB($scope.plan);
    var modelData = plansConversions.getViewedJson(nodes);
//  model = modelData;
//  $scope.model.edges = model.edges;
//  $scope.model.nodes = model.nodes;
//  $scope.model = model;
    setViewedPlan(modelData);
  };

  var setViewedPlan = function(modelData) {
    modelservice.selectAll();
    modelservice.deleteSelected();

    $scope.model = { nodes: [], edges: [] };

    $timeout(function() {
      for(var node of modelData.nodes) modelservice.nodes._addNode(node);
      for(var edge of modelData.edges) {
        console.log("Connecting: " + edge.source + " to " + edge.destination);
        modelservice.edges._addEdge(
          { id:edge.source,      type:flowchartConstants.bottomConnectorType },
          { id:edge.destination, type:flowchartConstants.topConnectorType }
        );
      }
      minConnectorID = 0;
      maxNodeID = Math.max(...model.nodes.map(function(node) {
        minConnectorID = Math.min(minConnectorID, ...modelservice.nodes.getConnectorIds(node));
        return node.id;
      }));
      $scope.model = model;
    }, 0);
  };

  $scope.savePlan = function() {
    var graph = plansConversions.getNodeFromView($scope.model);
    $scope.plan = plansConversions.convertToPlan(graph);

    if($scope.currentPlan != -1 && $scope.currentPlan !== undefined && !$scope.ctrlDown) {
      RheemAPI.updatePlan({id: $scope.currentPlan}, $scope.plan, function(data) {
        alert("plan updated successfully")
        $scope.currentPlan = $scope.plan;
        console.log("updated: " + data);
      });
    } else {
      $scope.ctrlDown = false;
      var planNames = $scope.plans.map(function(plan) { return plan.name; });
      do {
        var name = prompt("Please enter plan name", "Plan name");
        if(!name) {
          return;
        }
        if(planNames.indexOf(name) != -1) {
          name = "";
        }
      } while(!name);
      $scope.plan.name = name;
      RheemAPI.createPlan($scope.plan, function(data) {
        alert("plan created successfully");
        RheemAPI.getPlans({}, function(data) {
          $scope.plans = data;
          setSelectedPlan(name);
        });
      });
    }
  };

  var getSelectedPlan = function(id) {
    return $scope.plans.filter(function(plan) { return plan._id == id; })[0];
  };

  var setSelectedPlan = function(name) {
    $scope.currentPlan = $scope.plans.filter(function(plan) { return plan.name == name; })[0]._id;
  };

  $scope.openImport = function() {
    $scope.ctrlDown = false;

    var htmlTemplate =
      '<div class="modal-header"><h3>Import Plan</h3></div>' +
      '<div class="modal-body">' +
      '<textarea id="textareaJSON" rows=20 style="width:100%;"></textarea>' +
      '</div>' +
      '<div class="modal-footer">' +
      '<button class="btn btn-default" type="button" style="float:left;" onclick="this.nextSibling.click();">Upload from file</button>' +
      '<input type="file" style="display:none;" ng-model="model" onchange="var fr = new FileReader(); fr.onload = function(event) { document.getElementById(\'textareaJSON\').value = event.target.result; }; fr.readAsText(this.files[0]);">' +
      '<button class="btn btn-default" type="button" ng-click="importPlan();">Import</button> ' +
      '<button class="btn btn-default" type="button" ng-click="modalClose();">Cancel</button>' +
      '</div>';

    $scope.modalInstance = $uibModal.open({
      scope: $scope,
      size: 'lg',
      template: htmlTemplate
    });
  };

  $scope.importPlan = function() {
    var jsonPlan = document.getElementById('textareaJSON').value;
    $scope.modalInstance.dismiss('cancel');
    $scope.currentPlan = -1;
    $scope.plan = JSON.parse(jsonPlan);
    $scope.paramIndex = -1;
    $scope.selectedParam = [];
    $scope.selectedNodeParams = [];
    $scope.showParamPanel = false;
    var nodes = plansConversions.getNodesFromDB($scope.plan);
    var modelData = plansConversions.getViewedJson(nodes);
    setViewedPlan(modelData);
  };

// Not currently used...
//$scope.$on('modal.closing', function(event, reason, closed) {
//  // reason == $scope.modalInstance.dismiss(reason)
//});

  $scope.openExport = function() {
    $scope.ctrlDown = false;

    var viewedNodes = plansConversions.getNodeFromView($scope.model);
    var plan = plansConversions.convertToPlan(viewedNodes);
    var jsonPlan = JSON.stringify(plan, null, 4);

    var htmlTemplate =
      '<div class="modal-header"><h3>Export Plan</h3></div>' +
      '<div class="modal-body">' +
      '<textarea rows=20 readonly style="width:100%;">' + jsonPlan + '</textarea>' +
      '</div>' +
      '<div class="modal-footer">' +
      '<button class="btn btn-default" type="button" style="float:left;" onclick="this.nextSibling.click();">Download to file</button>' +
      '<a href="data:text/plain;charset=utf-8,' + encodeURIComponent(jsonPlan) + '" download="plan.json" style="display:none;"></a>' +
      '<button class="btn btn-default" type="button" ng-click="modalClose()">Close</button>' +
      '</div>';

    $scope.modalInstance = $uibModal.open({
      scope: $scope,
      size: 'lg',
      template: htmlTemplate
    });
  };

  $scope.modalClose = function() {
    $scope.modalInstance.dismiss('cancel');
  };

  // EXECUTION PLAN GRAPH
  $scope.exportJava = function() {
    if($scope.currentPlan != -1 && $scope.currentPlan !== undefined) {
      generatePlanForJava();
    } else {
      alert("no plan to generate java sample code")
    }
  };

  $scope.exportJson = function() {
    if($scope.currentPlan != -1 && $scope.currentPlan !== undefined) {
      if($scope.executedModel != -1 && $scope.executedModel !== undefined) {
        //alert(<pre>JSON.stringify($scope.executedModel)</pre>);
        var blob = new Blob([ angular.toJson($scope.plan) ], { type : 'application/json;charset=utf-8;' });
        $scope.url = (window.URL || window.webkitURL).createObjectURL( blob );
      } else {
        alert(generatePlan());
      }
    } else {
      alert("no plan to exportJson");
    }
  };

  var convertParametersToRheemStructure = function(plan) {
    for(var index in plan.operators) {
      var selectedConstructor = plan.operators[index].selectedConstructor;
      if((selectedConstructor == -1) && ("0" in plan.operators[index].parameters) && (Object.keys(plan.operators[index].parameters).length == 1)) {
        selectedConstructor = 0;
      }
      if(selectedConstructor != -1) {
        var rheemParam = {};
        for(var item in plan.operators[index].parameters[selectedConstructor]) {
          rheemParam[plan.operators[index].parameters[selectedConstructor][item].name] = plan.operators[index].parameters[selectedConstructor][item].value;
        }
        if($scope.simulate) {
          plan.operators[index].simulate = true;
        }
        plan.operators[index].parameters = rheemParam;
      }
    }
    console.log("RHEEM " + JSON.stringify(plan));
    return plan;
  };

  var convertRheemToGEMStructure = function(rheemPlan) {
    var copiedPlan = JSON.parse(JSON.stringify(rheemPlan));
    var gemPlan = copiedPlan.operators;
    for(var findex in gemPlan) {
      var op = gemPlan[findex];
      op.next = [];
      if(!("prev" in op)) {
        op.prev = [];
      }
      if("connects_to" in op) {
        for(var outnum = 0; outnum < op.np_outputs; outnum++) {
          for(var links of op.connects_to[outnum]) {
            for(var name in links) {
              for(var tindex in gemPlan) {
                if(name == gemPlan[tindex].name) {
                  op.next.push(tindex);
                  var target = gemPlan[tindex];
                  if(!("prev" in target)) {
                    target.prev = [];
                  }
                  target.prev.push(findex);
                }
              }
            }
          }
        }
        delete op.connects_to;
      }
    }
    console.log("GEM " + JSON.stringify(gemPlan));
    return(gemPlan);
  };

/*
// Not used - MDH 2019/01/16

  var generatePlan = function() {
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParametersToRheemStructure(copiedPlan);

    $scope.method = 'POST';
    $scope.url = '/api/rheem_plans/generate';
    $http({method: $scope.method, url: $scope.url, data: planWithRheemParams})
    .then(
      function(response) {
        console.log("build ", response);

        if(!response) {
          alert("No data has been return.");
        } else {
          var data = response.data;

          if(typeof data.error != 'undefined') {
            alert(data.error);
          } else {
            console.log("rheem_plans DATA ", JSON.stringify(data));
            $scope.executedModel  = data;
          }
        }
      },
      function(response) {
        alert("Unexpected error");
        console.log("rheem_plans error ", error);
      }
    );
  };

  var generatePlanForJava = function() {
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParametersToRheemStructure(copiedPlan);

    $scope.method = 'POST';
    $scope.url = '/rheem_plans/java';
    $http({method: $scope.method, url: $scope.url, data: planWithRheemParams})
    .then(
      function(response) {
        console.log("java build ", response);

        if(!response) {
          alert("No data has been return.");
        } else {
          var data = response.data;
          console.log("rheem_plans java data ", data);
        }
      },
      function(response) {
        alert("Unexpected error");
        console.log("rheem_plans java error ", error);
      }
    );
  };
*/

  // INFO PANEL
  $scope.showParamPanel = false;

  $scope.paramClicked = function(index) {
    $scope.paramIndex = index;
    $scope.model.nodes[$scope.model.nodes.indexOf($scope.selectedNodeData)].selectedConstructor = index;
    console.log($scope.model.nodes[$scope.model.nodes.indexOf($scope.selectedNodeData)]);
  };

  $scope.closeParamPanel = function() {
    $scope.showParamPanel = false;
  };

  var rebuildPlan = function() {

    // Export
    var viewedNodes = plansConversions.getNodeFromView($scope.model);
    var plan = plansConversions.convertToPlan(viewedNodes);

    var new_operators = [];
    var nodeIndex = {};
    for(var old_op of plan.operators) {
      // Only rebuild if operator class still exists, i.e. drop node if operator class no longer exists
      if(old_op.java_class in operatorIndex) {
        var nodetype = old_op.np_inputs ? old_op.np_outputs ? "operator" : "sink" : "datasource";
        var node = operatorIndex[old_op.java_class];
        var nodeClass =
          old_op.np_inputs  == 0 ? "source" :
          old_op.np_outputs == 0 ? "sink"   :
          old_op.np_inputs  == 1 ? "unary"  :
          old_op.np_inputs  == 2 ? "binary" : "trinary";
        var nodeName = old_op.name;

        var new_op = createNewNode(nodetype, node, nodeClass, nodeName);

        new_op.x = old_op.x;
        new_op.y = old_op.y;

        // Attempt to match and copy parameter values
        for(var old_param_set in old_op.parameters) {
          for(var old_param of old_op.parameters[old_param_set]) {
            for(var new_param_set in new_op.parameters) {
              for(var new_param of new_op.parameters[new_param_set]) {
                if((old_param.name == new_param.name) && (old_param.type == new_param.type)) {
                  new_param.value = old_param.value;
                }
              }
            }
          }
        }

        new_op.connects_to   = old_op.connects_to;
        new_op.broadcasts_to = operatorIndex[new_op.java_class].features.broadcast ? old_op.broadcasts_to : {};
        new_op.np_inputs     = operatorIndex[new_op.java_class].nb_inputs;
        new_op.np_outputs    = operatorIndex[new_op.java_class].nb_outputs;

        new_operators.push(new_op);
        nodeIndex[nodeName] = new_op;
      }
    };

    // Evaluate edges and remove any that connected to dropped nodes
    for(var new_op of new_operators) {
      if("connects_to" in new_op) {
        for(var out_nb in new_op.connects_to) {
          for(var i = 0; i < new_op.connects_to[out_nb].length; i++) {
            var edge = new_op.connects_to[out_nb][i];
            var count = 0;
            for(var nodeName in edge) {
              if((nodeName in nodeIndex) && (edge[nodeName] <= operatorIndex[nodeIndex[nodeName].java_class].nb_inputs - 1)) {
                count++;
              } else {
                delete edge[nodeName];
              }
            }
            if(count == 0) {
              new_op.connects_to[out_nb].splice(i--, 1);
            }
          }
        }
      }
    }

    plan.operators = new_operators;

    // Import
//  $scope.currentPlan = -1; // <-- Would allow/force resaving plan
    $scope.plan = plan;
    $scope.paramIndex = -1;
    $scope.selectedParam = [];
    $scope.selectedNodeParams = [];
    $scope.showParamPanel = false;
    var nodes = plansConversions.getNodesFromDB($scope.plan);
    var modelData = plansConversions.getViewedJson(nodes);
    setViewedPlan(modelData);

  }

  $scope.buildPlan = function() {
if($scope.ctrlDown) {
    rebuildPlan();
} else {
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParametersToRheemStructure(copiedPlan);

    $scope.method = 'POST';
    $scope.url = '/api/rheem_plans';
    $http({method: $scope.method, url: $scope.url, data: planWithRheemParams})
    .then(
      function(response) {
        console.log("build ", response);

        if(!response) {
          alert("No data has been return.");
        } else {
          var data = response.data;

          if(typeof data.error != 'undefined') {
            alert(data.error);
          } else {
            console.log("rheem_plans DATA ", JSON.stringify(data));
            alert("Your workflow is ready.");
            // $scope.executedModel  = data;
            // excutionPlan.set(plansConversions.getViewFromExec(data, 250, 100,
            //   {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"}));
            // plansConversions.set(data);
            // excutionPlan.drawExecutionPan();
            //
            // alert("Your workflow is ready.");
          }
        }
      },
      function(response) {
        alert("Unexpected error");
        console.log("rheem_plans error ", error);
      }
    );
}
  };

  $scope.switchPanelState = function() {
    console.log($scope.planSwitch);
    $scope.planSwitch = $scope.planSwitch;
  };

/*
// Not used - MDH 2019/01/16
  $scope.aceLoaded = function(_editor) {
    // Options
    _editor.setReadOnly(true);
  };

  $scope.aceChanged = function(e) {
    console.log("aceChanged: " + e);
  };

  $scope.saveParamData = function() {
  };
*/

  var callExecutePlan = function() {
    $scope.method = 'POST';
    $scope.url = '/api/plan_executions';
    return $http(
      {
        method: $scope.method,
        url: $scope.url,
        data: $scope.useGEM
              ? convertRheemToGEMStructure(plansConversions.get())
              : plansConversions.get()
      }
    ).then(
      function(response) {
        if(response && response.data && ("run_id" in response.data)) {
          $scope.run_id = response.data.run_id;
          ShowProgress(response);
          $timeout(getProgress, 2000);
          return;
        }

        if(response && response.data.myURI !== "") {
          console.log("Essam response.data.myURI:" + response.data.myURI);  
          console.log(response);
          window.open(response.data.myURI);
        }

        if(response) {
          var data = response.data;
          if(typeof data.error != 'undefined') {
            alert(data.error);
          } else {
            // $scope.monitorModel  = plansConversions.getViewFromExec(data, 250, 100, {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"});
            // $scope.drawMonitorGraph();
            // $interval(function() {
            //     $scope.updateProgress(data.run_id || 0);
            // }, 500);
          }
        } else {
          alert("No data has been returned");
        }
      },
      function(response) {
        // console.log("error ", error);
        console.log("Unexpected error on plan execution\r\n" + response.statusText);
        alert("Unexpected error on plan execution");
      }
    );
  };

// MDH - Remove 'ts' from $scope?

  $scope.executeClicked = function() {
    // Automatically deselect nodes and close param panel
    modelservice.deselectAll();
    $scope.showParamPanel = false;

    // Testing executing displayed plan...
    var viewedNodes = plansConversions.getNodeFromView($scope.model);
    $scope.plan  = plansConversions.convertToPlan(viewedNodes);

    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParametersToRheemStructure(copiedPlan);

    $scope.loader_req = true;
    plansConversions.set(planWithRheemParams);

    $scope.ts = (new Date()).getTime();
    console.log("started", $scope.ts);
    callExecutePlan()
    .then(
      function() {
        if($scope.run_id) {
          return;
        }
        var te = (new Date()).getTime();
        console.log("finished:", te, te - $scope.ts);
        $scope.loader_req = false;
        if((te - $scope.ts) > 300) {
          alert("Your data is ready.");
        }
      }
    );
  };

  const STATE_WAITING = 0;
  const STATE_BLOCKED = 1;
  const STATE_ACTIVE  = 2;
  const STATE_USER    = 3;
  const STATE_LOOP    = 4;
  const STATE_DONE    = 5;
  const STATE_ERROR   = 6;

  var states = ["Waiting", "Blocked", "Active", "User", "Loop", "Success", "Error"];

  function getProgress() {
    return $http(
      {
        method: "GET",
        url: '/api/progress?run_id=' + $scope.run_id
      }
    ).then(
      function(response) {
        ShowProgress(response);
        if(response.data.state == STATE_ACTIVE) {
          $timeout(getProgress, 2000);
        } else {
          var te = (new Date()).getTime();
          console.log("finished:", te, te - $scope.ts);
          $scope.loader_req = false;
          $scope.run_id = null;
          alert("Plan completion state: " + states[response.data.state]);
          ClearProgressBars();
        }
      },
      function(response) {
        alert("Error getting plan execution status");
      }
    );
  }

  function ClearProgressBars() {
    for(var i = 0; i < model.nodes.length; i++) {
      var node = model.nodes[i];
      node.rgbProgress = "0,0,0";
      node.pctProgress = 0;
      node.disProgress = "none";
    }
  }

  function ShowProgress(response) {
    var status = response.data;
    var colors = [
      "211,211,211", // waiting - lightgray
      "173,216,230", // blocked - lightblue
      "144,238,144", // active  - lightgreen
      "255,255,0",   // user    - yellow
      "173,216,230", // loop    -
      "0,0,0",       // done    - (never displayed)
      "255,192,203"  // error   - pink (i.e. light red)
    ];
    angular.forEach($scope.model.nodes, function(node) {
      var status_node = status.op_status[node.name];
      node.rgbProgress = colors[status_node.state];
      if(status_node.state != STATE_ACTIVE) {
        // If not executing, always show full color
        node.pctProgress = 100;
      } else {
        if(node.features && node.features.progress) {
          if(status_node.progress) {
            // Set percentage of fill based on progress
            node.pctProgress = (status_node.progress * 100) | 0;
          } else {
            // If progress is not yet set, assume initializing
            node.pctProgress = 0;
          }
        } else {
          // If operator does not support progress, show full color
          console.log("No progress for " + node.name);
          node.pctProgress = 100;
        }
      }
      node.disProgress = (status_node.state != STATE_DONE) ? "block" : "none";
    });
  }

}]);
