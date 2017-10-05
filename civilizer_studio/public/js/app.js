'use strict';

var rheem = angular.module('rheem', [
  'appControllers',
  'appDirectives',
  'appServices',
  'appFilters',
  'flowchart',
  'ui.router',
  'ui.bootstrap',
  'ui.ace'
]);

rheem.factory('prompt', function () {
    return prompt;
  })
  .config(function (NodeTemplatePathProvider) {
    NodeTemplatePathProvider.setTemplatePath("views/partials/node.html");
  });
var appControllers = angular.module('appControllers', []);
var appDirectives = angular.module('appDirectives', []);
var appServices = angular.module('appServices', ['ngResource']);
var appFilters = angular.module('appFilters', []);

rheem.config(['$qProvider', function ($qProvider) {
  $qProvider.errorOnUnhandledRejections(false);
}]);

rheem.config(function($stateProvider, $urlRouterProvider, $locationProvider, $compileProvider) {

  $compileProvider.aHrefSanitizationWhitelist(/^\s*(https?|ftp|mailto|tel|file|blob):/);

  $urlRouterProvider.otherwise('/');

  $stateProvider
    .state('main', {
      url: '/',
      templateUrl: 'views/partials/header.html',
      controller: 'mainController'
    })
    .state('main.editor', {
      url: 'editor',
      templateUrl: 'views/partials/editor.html',
      controller: 'editorController'
    })
    .state('main.monitor', {
      url: 'monitor',
      templateUrl: 'views/partials/monitor.html',
      controller: 'monitorController'
    })
  });