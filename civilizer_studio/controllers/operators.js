var Proxy = require('../helpers/proxy');
proxy = new Proxy();

function Operator() {

}

Operator.getByType = function(req, res){
    console.log(req.getallheaders);

    proxy.forwardRequest(req, res);


}

module.exports = Operator;
