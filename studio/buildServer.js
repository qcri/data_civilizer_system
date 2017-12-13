var express = require('express');
var app = express();
var bodyParser = require('body-parser');

app.use(bodyParser.json())


var json = {
           "stages":[
              {
                 "platform":"Apache Spark",
                 "operators":[
                    {
                       "is_terminal":0,
                       "is_start":1,
                       "connects_to":{
                          "0":[
                             {
                                "via":"RddChannel",
                                "sparkObjectFileSink":0
                             }
                          ]
                       },
                       "name":"sparkTextFileSource",
                       "java_class":"SparkTextFileSource"
                    },
                    {
                       "is_terminal":1,
                       "connects_to":{
                          "0":[
                             {
                                "javaObjectFileSource":0,
                                "via":"FileChannel"
                             }
                          ]
                       },
                       "java_class":"SparkObjectFileSink",
                       "name":"sparkObjectFileSink",
                       "is_start":0
                    }
                 ],
                 "sequence_number":0
              },
              {
                 "platform":"Java Streams",
                 "operators":[
                    {
                       "is_terminal":0,
                       "connects_to":{
                          "0":[
                             {
                                "via":"CollectionChannel",
                                "javaFlatMapOperator":0
                             }
                          ]
                       },
                       "java_class":"JavaObjectFileSource",
                       "name":"javaObjectFileSource",
                       "is_start":1
                    },
                    {
                       "is_terminal":0,
                       "connects_to":{
                          "0":[
                             {
                                "javaObjectFileSink":0,
                                "via":"CollectionChannel"
                             }
                          ]
                       },
                       "java_class":"JavaFlatMapOperator",
                       "name":"javaFlatMapOperator",
                       "is_start":0
                    },
                    {
                       "is_terminal":1,
                       "connects_to":{
                       },
                       "java_class":"JavaObjectFileSink",
                       "name":"javaObjectFileSink",
                       "is_start":0
                    }
                 ],
                 "sequence_number":1
              }
           ],
           "id":1,
           "rheem_plan_id":1,
           "name":"name"
        };

app.post('/executePlans/build', function (req, res) {
   if(Object.keys(req.body).length != 0){
    res.send(json);
   }else{
    res.send({});
   }
})


var server = app.listen(5010, function () {
  console.log("start");
});