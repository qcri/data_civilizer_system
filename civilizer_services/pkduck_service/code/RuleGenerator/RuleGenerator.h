//
// Created by Wenbo Tao on 12/26/16.
//

#ifndef XCLEAN_RULEGENERATOR_H
#define XCLEAN_RULEGENERATOR_H
#include "../Common.h"
using namespace std;


class RuleGenerator
{
public:
	//constructor
	RuleGenerator() {}
	RuleGenerator(vector<string>);

	//abstract method for generating rules
	virtual vector<t_rule> gen_rules() = 0;

	//virtual destructor
	virtual ~RuleGenerator() = default;

protected:
	//cells
	vector<string> cells;

	//number of cells
	int n;

	//token sets and maps
	vector<vector<string>> tokens;
	vector<umpsi> token_maps;

	//stop words
	const unordered_set<string> stop_words =
			unordered_set<string>({"the", "for", "to", "in", "and", "of", "on"});
};


#endif //XCLEAN_RULEGENERATOR_H
