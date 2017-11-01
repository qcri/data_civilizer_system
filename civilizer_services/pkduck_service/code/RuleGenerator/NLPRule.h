//
// Created by Wenbo Tao on 1/28/17.
//

#ifndef XCLEAN_NLPRULE_H
#define XCLEAN_NLPRULE_H


#include "RuleGenerator.h"
using namespace std;

class NLPRule : public RuleGenerator
{
public:
	//constructor
	NLPRule(vector<string>);

	//overwrite virtual method
	vector<t_rule> gen_rules();
};


#endif //XCLEAN_NLPRULE_H
