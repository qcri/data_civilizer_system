'use strict';

appControllers.controller('editorController', ['$scope', 'prompt', 'Modelfactory', 'flowchartConstants', 'RheemAPI', 'excutionPlan', '$http', 'plansConversions', function($scope, prompt, Modelfactory, flowchartConstants, RheemAPI, excutionPlan, $http, plansConversions) {




  // Drag Area Collapse Blocks
  $scope.isCollapsed = false;
  $scope.arrowDirection = true;

  $scope.paramIndex  = -1;

  $scope.soucresCollapseClicked = function () {
    $scope.sourcesIsCollapsed = !$scope.sourcesIsCollapsed;
  }
  $scope.unaryOperatorsCollapseClicked = function () {
    $scope.unaryOperatorsIsCollapsed = !$scope.unaryOperatorsIsCollapsed;
  }
  $scope.binaryOperatorsCollapseClicked = function () {
    $scope.binaryOperatorsIsCollapsed = !$scope.binaryOperatorsIsCollapsed;
  }
  $scope.trinaryOperatorsCollapseClicked = function () {
    $scope.trinaryOperatorsIsCollapsed = !$scope.trinaryOperatorsIsCollapsed;
  }
  $scope.sinkCollapseClicked = function () {
    $scope.sinkIsCollapsed = !$scope.sinkIsCollapsed;
  }
  $scope.confCollapseClicked = function () {
    $scope.confIsCollapsed = !$scope.confIsCollapsed;
  }

  $scope.unaryOperators = [];
  $scope.binaryOperators = [];
  $scope.trinaryOperators = [];
  $scope.datasources = [];
  $scope.sinks = [];

  var bindOperatorsData = function(operators){
    for(var operator of operators){
      if(operator.loop) {
        $scope.trinaryOperators.push(operator);
      } else if (operator.nb_inputs && operator.nb_inputs == 1 && operator.nb_outputs){
        $scope.unaryOperators.push(operator);
      } else if(operator.nb_inputs && operator.nb_inputs == 2 && operator.nb_outputs) {
        $scope.binaryOperators.push(operator);
      } else if(operator.nb_inputs && operator.nb_inputs > 2 && operator.nb_outputs) {
        $scope.trinaryOperators.push(operator);
      } else if(operator.nb_inputs && !operator.nb_outputs) {
        $scope.sinks.push(operator);
      } else if(operator.nb_outputs && !operator.nb_inputs) {
        $scope.datasources.push(operator);
      }
    }
  }

  // GET OPERATORS FROM API
  // RheemAPI.getBasicOperators({}, function(data){
  //   bindOperatorsData(data.operators);
  // });
  //
  // RheemAPI.getJavaOperators({}, function(data){
  //   bindOperatorsData(data.operators);
  // });
  //
  // RheemAPI.getSparkOperators({}, function(data){
  //   bindOperatorsData(data.operators);
  // });

  RheemAPI.getRheemOperator({}, function(data){
    bindOperatorsData(data.operators);
  });

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

  $scope.itemSelected = function(item){
  };

  // Drag & Drop
  var deleteKeyCode = 46;
  var ctrlKeyCode = 17;
  var aKeyCode = 65;
  var escKeyCode = 27;
  var nextNodeID = 10;
  var nextConnectorID = 20;
  var nextParamID = 30;
  var ctrlDown = false;
  var dragOptions = {
    placeholder: true
  }

  var model = {
    nodes: [],
    edges: []
  };

  $scope.nodeNames = [];
  $scope.flowchartselected = [];
  var modelservice = Modelfactory(model, $scope.flowchartselected);

  $scope.model = model;
  $scope.modelservice = modelservice;

  $scope.keyDown = function (evt) {
    if (evt.keyCode === ctrlKeyCode) {
      ctrlDown = true;
      evt.stopPropagation();
      evt.preventDefault();
    }
  };

  $scope.keyUp = function (evt) {
    if (evt.keyCode === deleteKeyCode) {
      modelservice.deleteSelected();
    }

    if (evt.keyCode == aKeyCode && ctrlDown) {
      modelservice.selectAll();
    }

    if (evt.keyCode == escKeyCode) {
      modelservice.deselectAll();
    }

    if (evt.keyCode === ctrlKeyCode) {
      ctrlDown = false;
      evt.stopPropagation();
      evt.preventDefault();
    }
  };
    var enterNodeName = function(nodename){
        var nodeName = prompt("Enter a node name:", nodename);
        console.log(nodeName);
        if( !nodeName ){
            return -1;
        }else if($scope.nodeNames.indexOf(nodeName) != -1){
            alert("Node name already exist")
            return enterNodeName();
        } else {
            $scope.nodeNames.push(nodeName);
            return nodeName;
        }
    }
  $scope.addNewNode = function (nodetype,  node, nodeClass) {
    node = JSON.parse(JSON.stringify(node));
    console.log(node);
    var nodeName = enterNodeName( node.class.split(".").splice(-1)[0] );
    if(nodeName == -1)
        return;
        var getNodeParams = function(parameters){
          for (var item in parameters) {
            for (var paramsItem in parameters[item]) {
              if (parameters[item][paramsItem].inputType == 'UDF') {
                var udfTmpl = parameters[item][paramsItem].udfTmpl;
                parameters[item][paramsItem].value = udfTmpl.replace(/OPNAME/g, nodeName)
                                                            .replace(/PARAMNAME/g, parameters[item][paramsItem].name);
              }else {
                parameters[item][paramsItem].value = "";
              }
              parameters[item][paramsItem].id = nextParamID++;
            }
          }
          return parameters;
        }

      var getColorCodeByType = function(nodetype){
        if (nodetype == "sink"){
          return '#F15B26';
        } else if(nodetype == "datasource"){
          return '#F15B29';
        } else if(nodetype == "operator"){
          return '#F15B30';
        }
      }

      var getConnectorsByType = function(nodetype, node){
        var connectorsArr = [];
        if(nodetype == "sink"){
          var sinkNode = {
            id: nextConnectorID++,
            type: flowchartConstants.topConnectorType
          }
          connectorsArr.push(sinkNode);
        }else if(nodetype == "datasource"){
          var datasourceNode = {
            id: nextConnectorID++,
            type: flowchartConstants.bottomConnectorType
          }
          connectorsArr.push(datasourceNode);
        }else if(nodetype == "operator"){
          if(node.nb_inputs >= 1){
            var inputPortOperatorNode = {
              id: nextConnectorID++,
              type: flowchartConstants.topConnectorType
            }
            connectorsArr.push(inputPortOperatorNode);
          }

          if(node.nb_inputs >= 2){
            var inputPortOperatorNode = {
              id: nextConnectorID++,
              type: flowchartConstants.topConnectorType
            }
            connectorsArr.push(inputPortOperatorNode);
          }

          if(node.nb_inputs >= 3){
            var inputPortOperatorNode = {
              id: nextConnectorID++,
              type: flowchartConstants.topConnectorType
            }
            connectorsArr.push(inputPortOperatorNode);
          }

          var outputPortOperatorNode = {
            id: nextConnectorID++,
            type: flowchartConstants.bottomConnectorType
          }
          connectorsArr.push(outputPortOperatorNode);

          if(node.nb_inputs >= 2){
             var outputPortOperatorNode = {
              id: nextConnectorID++,
              type: flowchartConstants.bottomConnectorType
            }
            connectorsArr.push(outputPortOperatorNode);
          }

          if(node.supportBroadcast){
            var rightNode = {
              id: nextConnectorID++,
              type: flowchartConstants.topConnectorType,
              broadcast: true
            }
            connectorsArr.push(rightNode);

            var leftNode = {
              id: nextConnectorID++,
              type: flowchartConstants.topConnectorType,
              broadcast: true
            }
            connectorsArr.push(leftNode);
          }

        }

        return connectorsArr;
      }

      var connectors = getConnectorsByType(nodetype, node);
      var colorCode = getColorCodeByType(nodetype);
      var newNode = {
        name: nodeName,
        id: nextNodeID++,
        x: 10,
        y: 10,
        color: colorCode,
        connectors: connectors,
        java_class: node.class,
        parameters: getNodeParams(node.parameters),
        type: nodeClass,
        selectedConstructor: -1,
        isBroadCast: node.supportBroadcast
      };

      model.nodes.push(newNode);
  };

  $scope.activateWorkflow = function() {
    angular.forEach($scope.model.edges, function(edge) {
      edge.active = !edge.active;
    });
  };

  $scope.addNewInputConnector = function () {
    var connectorName = prompt("Enter a connector name:", "New connector");
    if (!connectorName) {
      return;
    }

    var selectedNodes = modelservice.nodes.getSelectedNodes($scope.model);
    for (var i = 0; i < selectedNodes.length; ++i) {
      var node = selectedNodes[i];
      node.connectors.push({id: nextConnectorID++, type: flowchartConstants.topConnectorType});
    }
  };

  $scope.addNewOutputConnector = function () {
    var connectorName = prompt("Enter a connector name:", "New connector");
    if (!connectorName) {
      return;
    }

    var selectedNodes = modelservice.nodes.getSelectedNodes($scope.model);
    for (var i = 0; i < selectedNodes.length; ++i) {
      var node = selectedNodes[i];
      node.connectors.push({id: nextConnectorID++, type: flowchartConstants.bottomConnectorType});
    }
  };

  $scope.deleteSelected = function () {
    modelservice.deleteSelected();
  };

  $scope.callbacks = {
    edgeDoubleClick: function () {
      //console.log('Edge double clicked.');
    },
    edgeMouseOver: function () {
      //console.log('mouserover')
    },
    isValidEdge: function (source, destination) {
      return source.type === flowchartConstants.bottomConnectorType && destination.type === flowchartConstants.topConnectorType;
    },

    nodeCallbacks: {
      'singleClick': function (item) {

      },
      'doubleClick': function (item) {
        $scope.item = item;
        $scope.selectedNodeData = item;
        $scope.selectedParam = [];
        $scope.selectedNodeParams = [];
        for ( var x in $scope.selectedNodeData.parameters) {
          var paramStr = "(";
          for (var y in $scope.selectedNodeData.parameters[x] ) {
            paramStr += $scope.selectedNodeData.parameters[x][y].type + ",\n";
          }
          paramStr = paramStr.trim().slice(0, -1);
          paramStr += ")";
          $scope.selectedNodeParams.push(paramStr);
        }
        if(item.selectedConstructor != -1){
        //  $scope.selectedParam =$scope.selectedNodeData.parameters[item.selectedConstructor];
          $scope.paramIndex = item.selectedConstructor;
          $scope.selectedParam = $scope.model.nodes[$scope.model.nodes.indexOf($scope.selectedNodeData)].parameters[item.selectedConstructor];
        }else {
          $scope.paramIndex = -1;
          $scope.selectedParam = [];
        }
        $scope.showParamPanel = true;
      }
    }
  };

    // INFO PANEL
    $scope.showParamPanel = false;

     //Plans
     //var person = prompt("Please enter plan name", "Plan name");
     $scope.currentPlan = -1;

    RheemAPI.getPlans({}, function(data){
        $scope.plans = data;
    })

    $scope.showCurrentPlan = function() {
      console.log($scope.currentPlan);
      $scope.plan = getSelectedPlan($scope.currentPlan)[0];
      console.log($scope.plan);
      $scope.paramIndex = -1;
      $scope.selectedParam = [];
      $scope.selectedNodeParams = [];
      $scope.showParamPanel = false;
      var nodes = plansConversions.getNodesFromDB($scope.plan);
      var modelData = plansConversions.getViewedJson(nodes);
      model = modelData;
      $scope.model.edges = model.edges;
      $scope.model.nodes = model.nodes;
      $scope.model = model;
    }

    var convertParamtersToRheemStructure = function(plan) {
      var rheemParam = {};
      for (var index in plan.operators) {
        if(plan.operators[index].selectedConstructor != -1){
          for (var item in plan.operators[index].parameters[plan.operators[index].selectedConstructor]) {
            rheemParam[plan.operators[index].parameters[plan.operators[index].selectedConstructor][item].name] =  plan.operators[index].parameters[plan.operators[index].selectedConstructor][item].value;
          }
          plan.operators[index].parameters = rheemParam;
          rheemParam = {};
        }
      }
      return plan;
    }

    $scope.savePlan = function(){
        var graph = plansConversions.getNodeFromView($scope.model);
        $scope.plan  = plansConversions.convertToPlan(graph);

        if($scope.currentPlan != -1 && $scope.currentPlan !== undefined){
          RheemAPI.updatePlan({id: $scope.currentPlan}, $scope.plan, function(data){
            alert("plan updated successfully")
            $scope.currentPlan = $scope.plan;
            console.log("updated: " + data);

          });
        }else{
          $scope.plan.name = prompt("Please enter plan name", "Plan name");
          RheemAPI.createPlan($scope.plan, function(data){
              alert("plan created successfully");
              window.location.reload();
          });
        }

    }

    $scope.createNewPlan = function(){
        window.location.reload();
    }

    var getSelectedPlan = function(id){
        return $scope.plans.filter(function(plan){
            return plan._id == id;
        });
    }

  // EXECUTION PLAN GRAPH
  $scope.exportJava = function(){
    if($scope.currentPlan != -1 && $scope.currentPlan !== undefined) {
      generatePlanForJava();
    }
    else{
      alert("no plan to generate java sample code")
    }
  }

  $scope.exportJson = function(){
    if($scope.currentPlan != -1 && $scope.currentPlan !== undefined) {
      if($scope.executedModel != -1 && $scope.executedModel !== undefined){
        //alert(<pre>JSON.stringify($scope.executedModel)</pre>);
        var blob = new Blob([ angular.toJson($scope.plan) ], { type : 'application/json;charset=utf-8;' });
        $scope.url = (window.URL || window.webkitURL).createObjectURL( blob );
      }
      else {
        alert(generatePlan());
      }
    }
    else{
      alert("no plan to exportJson")
    }
  }


  var generatePlan = function() {
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParamtersToRheemStructure(copiedPlan);

    $scope.method = 'POST';
    $scope.url = '/api/rheem_plans/generate';
    $http({method: $scope.method, url: $scope.url, data: planWithRheemParams}).
        then(function(response) {
          console.log("build ", response);

          if(!response){
            alert("No data has been return.");
          }
          else{

            var data = response.data;

            if(typeof data.error != 'undefined'){
              alert(data.error);
            }
            else{
              console.log("rheem_plans DATA ", JSON.stringify(data));
              $scope.executedModel  = data;
            }
          }
        }, function(response) {
          alert("Unexpected error");
          console.log("rheem_plans error ", error);

        });

  }


  var generatePlanForJava = function() {
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParamtersToRheemStructure(copiedPlan);

    $scope.method = 'POST';
    $scope.url = '/rheem_plans/java';
    $http({method: $scope.method, url: $scope.url, data: planWithRheemParams}).
        then(function(response) {
          console.log("java build ", response);

          if(!response){
            alert("No data has been return.");
          }
          else{
            var data = response.data;
            console.log("rheem_plans java data ", data);

          }
        }, function(response) {
          alert("Unexpected error");
          console.log("rheem_plans java error ", error);

        });

  }












  // INFO PANEL
  $scope.showParamPanel = false;
  $scope.paramClicked = function(index) {
    $scope.paramIndex = index;
    $scope.model.nodes[$scope.model.nodes.indexOf($scope.selectedNodeData)].selectedConstructor = index;
    console.log($scope.model.nodes[$scope.model.nodes.indexOf($scope.selectedNodeData)]);
  }

  $scope.closeParamPanel = function() {
    $scope.showParamPanel = false;
  }

  $scope.buildPlan = function() {
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParamtersToRheemStructure(copiedPlan);

    $scope.method = 'POST';
    $scope.url = '/api/rheem_plans';
    $http({method: $scope.method, url: $scope.url, data: planWithRheemParams}).
        then(function(response) {
          console.log("build ", response);

          if(!response){
              alert("No data has been return.");
          }
          else{
            
            var data = response.data;

            if(typeof data.error != 'undefined'){
              alert(data.error);
            }
            else{
              console.log("rheem_plans DATA ", JSON.stringify(data));
              alert("Your workflow is ready.");
              // $scope.executedModel  = data;
              // excutionPlan.set(plansConversions.getViewFromExec(data, 250, 100,
              //     {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"}));
              // plansConversions.set(data);
              // excutionPlan.drawExecutionPan();
              //
              // alert("Your workflow is ready.");
            }
          }
        }, function(response) {
          alert("Unexpected error");
          console.log("rheem_plans error ", error);

        });

  }

  $scope.switchPanelState = function() {
    console.log($scope.planSwitch);
    $scope.planSwitch = $scope.planSwitch;
  }

  $scope.aceLoaded = function(_editor) {
    // Options
    _editor.setReadOnly(true);
  };

  $scope.aceChanged = function(e) {
    console.log("aceChanged: " + e);
  };

  $scope.saveParamData = function(){

  }


  $scope.callExecutePlan = function(){
        $scope.method = 'POST';
        $scope.url = '/api/plan_executions';
        return $http({method: $scope.method, url: $scope.url, data:plansConversions.get()}).
        then(function(response) {
            if(response){
                var data = response.data;
                if(typeof data.error != 'undefined'){
                    alert(data.error);
                }
                else{
                    // $scope.monitorModel  = plansConversions.getViewFromExec(data, 250, 100, {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"});
                    // $scope.drawMonitorGraph();
                    // $interval(function() {
                    //     $scope.updateProgress(data.run_id || 0);
                    // }, 500);

                }
            }
            else{
                alert("No data has been returned");
            }
        }, function(response) {
            console.log("error ", error);
            alert("Unexptected rror on plan execution");
        });
    }

  $scope.executeClicked = function(){
    var copiedPlan = JSON.parse(JSON.stringify($scope.plan));
    var planWithRheemParams = convertParamtersToRheemStructure(copiedPlan);

    $scope.loader_req = true;
    plansConversions.set(planWithRheemParams);
    $scope.callExecutePlan()
    .then(function(){
      $scope.loader_req = false;
        alert("Your data is ready.");
    })

      // vafr myVar = setInterval(myTimer, 1000);



  }
}]);
