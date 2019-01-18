appServices.factory('RheemAPI', ['$resource',
    function($resource) {
      return $resource('', {}, {
        getBasicOperators: {
            method: 'GET',
            url: '/api/operators/basic',
            cache: true
        },
        getSparkOperators: {
            method: 'GET',
            url: '/api/operators/spark',
            cache: true
        },
        getJavaOperators: {
            method: 'GET',
            url: '/api/operators/java',
            cache: true,
        },
        getPlans: {
            method: 'GET',
            url: '/api/plans',
            isArray: true,
            cache: false,
        },
        getPlanById: {
            method: 'GET',
            url: '/api/plans/:id',
            cache: true
        },
        getPlanByName: {
            method: 'GET',
            url: '/api/plans?name__regex=:name',
            isArray: true,
            cache: true
        },
        createPlan: {
            method: "POST",
            url: 'api/plans',
            cache: false
        },
        buildPlan: {
            method: "POST",
            url: 'api/plans/rheem_plans',
            cache: false
        },
        updatePlan: {
            method: "PUT",
            url: 'api/plans/:id',
            cache: false
        },
        deletePlan: {
            method: "DELETE",
            url: 'api/plans/:id',
            cache: false
        },
        getRheemOperator: {
            method: "GET",
            url: 'api/rheem_operators',
            // url: 'civilizer/operators',
            cache: true
        },
        getRheemJava: {
          method: "POST",
          url: 'api/plans/rheem_plans/java',
          cache: true
        }

      })
    }
]);
