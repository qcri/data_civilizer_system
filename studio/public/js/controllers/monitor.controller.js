'use strict';

appControllers.controller('monitorController', ['$scope', 'plansConversions', '$http', '$interval', function($scope, plansConversions, $http, $interval) {

  $scope.monitorModel = {}

  $scope.test = plansConversions.get();

//  $scope.monitorModel  = plansConversions.getViewFromExec($scope.test, 250, 100, {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"});
  //  excutionPlan.set(plansConversions.getViewFromExec(data, 250, 100, {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"}));
    //plansConversions.set(data);
  //  $scope.drawMonitorGraph();
  $scope.drawMonitorGraph = function() {
    var canvas = document.getElementById("monitor-graph");
    var ctx = canvas.getContext("2d");

    // SET CANVAS DIMENSIONS
    var nodesXs = []; var nodesYs = [];
    for (var node in $scope.monitorModel.nodes) {
      nodesXs.push($scope.monitorModel.nodes[node].x);
      nodesYs.push($scope.monitorModel.nodes[node].y);
    }
    var largestX = Math.max.apply(Math, nodesXs);
    var largestY = Math.max.apply(Math, nodesYs);
    $("#monitor-graph").attr('width', largestX + 200);
    $("#monitor-graph").attr('height', largestY + 100);

    // Draw Edges
    for (var edge in $scope.monitorModel.edges) {
      var edgeSource = $scope.monitorModel.edges[edge].source;
      var edgeSourceX = $scope.monitorModel.nodes[edgeSource].x;
      var edgeSourceY = $scope.monitorModel.nodes[edgeSource].y;

      var edgeTarget = $scope.monitorModel.edges[edge].target;
      var edgeTargetX = $scope.monitorModel.nodes[edgeTarget].x;
      var edgeTargetY = $scope.monitorModel.nodes[edgeTarget].y;

      var strokeLine = function(ctx, edgeSourceX, edgeSourceY, edgeTargetX, edgeTargetY, color){
        ctx.setLineDash([5, 8]);
        ctx.beginPath();
        ctx.moveTo(edgeSourceX + 100, edgeSourceY + 25); // start point
        ctx.lineTo(edgeTargetX + 100, edgeTargetY + 25);
        ctx.lineWidth = 3;
        ctx.strokeStyle = color;
        ctx.stroke();
      }

      switch( $scope.monitorModel.edges[edge].type ) {
        case "RddChannel":
          strokeLine(ctx, edgeSourceX, edgeSourceY, edgeTargetX, edgeTargetY, 'brown');
          break;
        case "FileChannel":
          strokeLine(ctx, edgeSourceX, edgeSourceY, edgeTargetX, edgeTargetY, 'orange');
          break;
        case "CollectionChannel":
          strokeLine(ctx, edgeSourceX, edgeSourceY, edgeTargetX, edgeTargetY, 'green');
          break;
        default:
          strokeLine(ctx, edgeSourceX, edgeSourceY, edgeTargetX, edgeTargetY, 'gray');
      }
    }

    // Draw Nodes
    for (var node in $scope.monitorModel.nodes){
      ctx.fillStyle = $scope.monitorModel.nodes[node].color;
      ctx.fillRect($scope.monitorModel.nodes[node].x, $scope.monitorModel.nodes[node].y, 200, 50);

      // Draw titles
      ctx.font = "18px Arial";
      ctx.fillStyle = "#FFF";
      ctx.textAlign="center";
      ctx.textBaseline="middle";
      ctx.fillText(node, $scope.monitorModel.nodes[node].x + 100, $scope.monitorModel.nodes[node].y + 25);

      // Draw progress
      var centerX = $scope.monitorModel.nodes[node].x + 175;
      var centerY = $scope.monitorModel.nodes[node].y + 50;
      var radius = 15;
      ctx.beginPath();
      ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI, false);
      ctx.fillStyle = '#333';
      ctx.fill();

      ctx.font = "12px Arial";
      ctx.fillStyle = "#FFF";
      ctx.fillText($scope.monitorModel.nodes[node].progress+"%", $scope.monitorModel.nodes[node].x + 175 , $scope.monitorModel.nodes[node].y + 50);
    }

  };

  //  $scope.drawMonitorGraph();
  if(plansConversions.get() == undefined || plansConversions.get() == null || plansConversions.get() == {}){

    $scope.method = 'GET';
    $scope.url = '/api/latest_plan_execution';
    $http({method: $scope.method, url: $scope.url}).
        then(function(data) {
          if(data){
            if(typeof data.error != 'undefined'){
              alert(data.error);
            }
            else{
              $scope.monitorModel  = plansConversions.getViewFromExec(data, 250, 100, {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"});
              $scope.drawMonitorGraph();
              $interval(function() {
                $scope.updateProgress(data.run_id || 0);
              }, 500);
            }
          }
          else{
            alert("No data has been returned.");
          }

        }, function(data) {
          console.log("error ", error);
          alert("Unexpected error on latest plan execution");
        });

  } else {

    $scope.method = 'POST';
    $scope.url = '/api/plan_executions';
    $http({method: $scope.method, url: $scope.url, data:plansConversions.get()}).
        then(function(response) {
          if(response){
            var data = response.data;
            if(typeof data.error != 'undefined'){
              alert(data.error);
            }
            else{
              $scope.monitorModel  = plansConversions.getViewFromExec(data, 250, 100, {"Java Streams": "brown", "Apache Spark": "orange", "Apache Hadoop": "green"});
              $scope.drawMonitorGraph();
              $interval(function() {
                $scope.updateProgress(data.run_id || 0);
              }, 500);
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


  $scope.monitorProgress = 0;

  $scope.updateProgress = function(id){

    $scope.method = 'GET';
    $scope.url = '/api/progress?run_id=' + id;
    console.log("$scope.url: ", $scope.url);
    $http({method: $scope.method, url: $scope.url}).
        then(function(response) {
          if(!response){
            alert("Error on progressing");
          }
          var data = response.data;
          if(typeof data.error != 'undefined'){
            alert("Error from server");
          }
          else{
              $scope.monitorProgress = data.overall;

              for(name in data["details"]){
                //$scope.monitorModel.nodes[name].progress = $scope.getRandomInt(0,100);
                $scope.monitorModel.nodes[name].progress = data["details"][name];
              }
              $scope.clearMonitorGraph();
              $scope.drawMonitorGraph();
              $scope.$apply();
          }
        }, function(response) {
          alert("Unexpected error");
          console.log("rheem_plans error ", error);

        });
  }


 $scope.getRandomInt = function (min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min)) + min;
  };

  $scope.clearMonitorGraph = function(){
    var canvas = document.getElementById("monitor-graph");
    var ctx = canvas.getContext("2d");
    ctx.clearRect(0, 0, canvas.width, canvas.height);
  };

}]);
