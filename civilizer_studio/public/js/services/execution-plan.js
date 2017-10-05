appServices.factory('excutionPlan', function(){

  var executionGraph = {};

  var set = function(model){
    executionGraph = model;
  }
  // var executionGraph = {
  //   nodes: {
  //     Source: {
  //       color: 'brown',
  //       y: 0,
  //       x: 100,
  //       progress: 0
  //     },
  //     'Operator #3': {
  //       color: 'brown',
  //       y: 400,
  //       x: 100
  //     },
  //     Sink: {
  //       color: 'brown',
  //       y: 500,
  //       x: 100
  //     },
  //     'Operator #1': {
  //       color: 'green',
  //       y: 100,
  //       x: 0
  //     },
  //     'Operator #1-1': {
  //       color: 'green',
  //       y: 200,
  //       x: 0
  //     },
  //     'Operator #2': {
  //       color: 'orange',
  //       y: 100,
  //       x: 200
  //     },
  //     'Operator #2-1': {
  //       color: 'orange',
  //       y: 200,
  //       x: 200
  //     },
  //     'Operator #2-2': {
  //       color: 'orange',
  //       y: 300,
  //       x: 200
  //     }
  //   },
  //   edges: [{
  //       source: 'Source',
  //       target: 'Operator #1',
  //       type: 'FileChannel'
  //     },
  //     {
  //       source: 'Source',
  //       target: 'Operator #2',
  //       type: 'FileChannel'
  //     },
  //     {
  //       source: 'Operator #3',
  //       target: 'Sink',
  //       type: 'RddChannel'
  //     },
  //     {
  //       source: 'Operator #1',
  //       target: 'Operator #1-1',
  //       type: 'CollectionChannel'
  //     },
  //     {
  //       source: 'Operator #1-1',
  //       target: 'Operator #3',
  //       type: 'CollectionChannel'
  //     },
  //     {
  //       source: 'Operator #2',
  //       target: 'Operator #2-1',
  //       type: 'CollectionChannel'
  //     },
  //     {
  //       source: 'Operator #2-1',
  //       target: 'Operator #2-2',
  //       type: 'CollectionChannel'
  //     },
  //     {
  //       source: 'Operator #2-2',
  //       target: 'Operator #3',
  //       type: 'CollectionChannel'
  //     }]
  // }

  return {
    set : set,
    drawExecutionPan: function(){

      var canvas = document.getElementById("execution-plan-graph");
      var ctx = canvas.getContext("2d");

      // SET CANVAS DIMENSIONS
      var nodesXs = []; var nodesYs = [];
      for (node in executionGraph.nodes) {
        nodesXs.push(executionGraph.nodes[node].x);
        nodesYs.push(executionGraph.nodes[node].y);
      }
      var largestX = Math.max.apply(Math, nodesXs);
      var largestY = Math.max.apply(Math, nodesYs);
      $("#execution-plan-graph").attr('width', largestX + 200);
      $("#execution-plan-graph").attr('height', largestY + 100);


      // DRAW EDGES
      for (edge in executionGraph.edges) {
        var edgeSource = executionGraph.edges[edge].source;
        var edgeSourceX = executionGraph.nodes[edgeSource].x;
        var edgeSourceY = executionGraph.nodes[edgeSource].y;

        var edgeTarget = executionGraph.edges[edge].target;
        var edgeTargetX = executionGraph.nodes[edgeTarget].x;
        var edgeTargetY = executionGraph.nodes[edgeTarget].y;

        var strokeLine = function(ctx, edgeSourceX, edgeSourceY, edgeTargetX, edgeTargetY, color){
          ctx.setLineDash([5, 8]);
          ctx.beginPath();
          ctx.moveTo(edgeSourceX + 100, edgeSourceY + 25); // start point
          ctx.lineTo(edgeTargetX + 100, edgeTargetY + 25);
          ctx.lineWidth = 3;
          ctx.strokeStyle = color;
          ctx.stroke();
        }

        switch( executionGraph.edges[edge].type ) {
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
      

      // DRAW NODES
      for (node in executionGraph.nodes){
        ctx.fillStyle = executionGraph.nodes[node].color;
        ctx.fillRect(executionGraph.nodes[node].x, executionGraph.nodes[node].y, 200, 50);

        // Draw titles
        ctx.font = "18px Arial";
        ctx.fillStyle = "#FFF";
        ctx.textAlign="center";
        ctx.textBaseline="middle";
        ctx.fillText(node, executionGraph.nodes[node].x + 100, executionGraph.nodes[node].y + 25);
      }
    }
  }
});
