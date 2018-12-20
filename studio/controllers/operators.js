var Proxy = require('../helpers/proxy');
proxy = new Proxy();

function Operator() {

}

Operator.getByType = function(req, res){
     var target = req.method + " " + process.env.API_SERVER_URL + "/rheem" + req.url;
    console.log(target + " @ " + new Date().toISOString());
    // console.log(req.getallheaders);
    // console.log(JSON.stringify(req.headers, null, 4));

    // Catch proxy errors if the backend is down...
    proxy.proxy.on('error', function (err, req, res) {
        console.log("Proxy error on " + target + "\n" + JSON.stringify(err, null, 4));
        return res.end();
    });

    proxy.forwardRequest(req, res);
}

module.exports = Operator;
