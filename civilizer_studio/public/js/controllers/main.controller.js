'use strict';
appControllers.controller('mainController', ['$scope', 'prompt', 'Modelfactory', 'flowchartConstants', 'RheemAPI', 'excutionPlan', '$http', 'plansConversions', function($scope, prompt, Modelfactory, flowchartConstants, RheemAPI, excutionPlan, $http, plansConversions) {

  $scope.planSwitch = false;

  $scope.buildClicked = function(){
    $scope.workAreaClass = 'col-xs-5';
  };

  $scope.switchPanelState = function(){
    console.log( $scope.planSwitch);
    $scope.planSwitch = $scope.planSwitch;
  }
  $scope.$watch('planSwitch', function() {
    if($scope.planSwitch){
      console.log(plansConversions.get());
    }
  })


}]);
