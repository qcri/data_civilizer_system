var restful = require('node-restful');
var mongoose = restful.mongoose;
var autoIncrement = require('mongoose-auto-increment');

autoIncrement.initialize(connection);

// Schema
Plan.schema = new mongoose.Schema({
	name: { type : String , required : true },
	created_at: { type: Date, default: Date.now },
	registered_platforms: {type : Array},
	sink_operators: {type : Array},
	operators: {type : Array},
	exec_id: {type: String},
	config: {type : Array}
});

//attach autoIncrement
Plan.schema.plugin(autoIncrement.plugin, 'plan');

Plan.model = mongoose.model('plan', Plan.schema);

//constructor

function Plan(){
}

module.exports = restful.model('Plans', Plan.schema);
