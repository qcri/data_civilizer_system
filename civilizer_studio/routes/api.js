//load shared  router
var express = require('express');
router = express.Router();

//load controllers
var Operator = require('../controllers/operators');
var Plans =  require('../controllers/plans');
var ExecutePlan = require('../controllers/executePlans');

//Routes
/* PLANS ROUTES */
router.use('/plans', Plans);
///////////////////////////////////////////////////////////////////////////////

/* Operators  Routes */
router.get("/rheem_operators", Operator.getByType);
///////////////////////////////////////////////////////////////////////////////

/* ExecutePlans  Routes */
router.use("", ExecutePlan);
///////////////////////////////////////////////////////////////////////////////

module.exports = router;
