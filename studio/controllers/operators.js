var Proxy = require('../helpers/proxy');
proxy = new Proxy();

function Operator() {

}

Operator.getByType = function(req, res){
    console.log(req.method + " " + process.env.API_SERVER_URL + "/rheem" + req.url + " @ " + new Date().toISOString());
    // console.log(req.getallheaders);
    // console.log(JSON.stringify(req.headers, null, 4));

    try {
        proxy.forwardRequest(req, res);
    }
    catch(e) {
        return res.json([])
    }
}

module.exports = Operator;
