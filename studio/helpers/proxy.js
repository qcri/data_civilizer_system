var httpProxy = require('http-proxy');

function Proxy () {
    if (!this.proxy) {
        this.proxy = httpProxy.createProxyServer();
    }
}

//member functions
Proxy.prototype = {
    forwardRequest: function(req, res) {
      this.proxy.proxyRequest(req, res, {
          changeOrigin: true,
          target: config.apiServer + '/rheem'
      });
    }
}

module.exports = Proxy;
