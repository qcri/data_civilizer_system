var Plan = require('../models/plan');
var Proxy = require('../helpers/proxy');
proxy = new Proxy();

Plan.methods(['get', 'post', 'put', 'delete']);
Plan.route("execute.post",{detail: false, handler: function(req,res,next) {
    proxy.forwardRequest(req, res);
}});

Plan.route("rheem_plans.post", function(req,res,next) {
  ///console.log(req.body);
    proxy.forwardRequest(req, res);
});

Plan.route("history.get",{detail: false, handler: function(req,res,next) {
    proxy.forwardRequest(req, res);
}});

Plan.register(router, '/plans');

module.exports = router;
