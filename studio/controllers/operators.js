var Proxy = require('../helpers/proxy');
proxy = new Proxy();

function Operator() {

}

Operator.getByType = function(req, res){
    console.log(req.method + " " + process.env.API_SERVER_URL + "/rheem" + req.url + " @ " + new Date().toISOString());
    // console.log(req.getallheaders);
    // console.log(JSON.stringify(req.headers, null, 4));

    proxy.forwardRequest(req, res);


}

module.exports = Operator;
