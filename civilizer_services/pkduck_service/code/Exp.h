//
// Created by Wenbo Tao on 2/11/17.
//

#ifndef XCLEAN_EXP_H
#define XCLEAN_EXP_H
#include "Common.h"
#include "Solver/Solver.h"

using namespace std;


class Exp
{
public:
	// datasets
	static unordered_set<string> file_names;

	// preprocess the raw data
	static void preprocess();

	// check
	static void check();
	static void check2();

	// effectiveness
	static void varyDictionary();
	static void varyMeasure();

	// #candidate & signature generation time
	static void varyThreshold();

	// run solver
	static void runSolver();

	//calculate Measure PRF
	static void calculateMeasurePRF();

	//calculate dictionary PRF
	static void calculateDictPRF();

	//generate dirty records
	static void genDirty();

	//dictionary runtime scalability
	static void dictionary_scale();

	//join algorithm scalability
	static void joinalgo_scale();

	//dataset details
	static void show_datasets();

	//rule compression
	static void testRuleCompression();

	//generate smaller subset
	static void genSubset();

	//generate data for loading to db
	static void genDBData(double);
};


#endif //XCLEAN_EXP_H
