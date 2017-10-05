appFilters.filter('prettyClassName', function() {
  return function(input) {
    var arr = input.split('.');
    return arr[arr.length - 1];  
  }
});