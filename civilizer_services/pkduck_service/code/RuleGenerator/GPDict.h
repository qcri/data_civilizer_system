//
// Created by Wenbo Tao on 6/18/17.
//

#ifndef XCLEAN_GPDICT_H
#define XCLEAN_GPDICT_H

#include "RuleGenerator.h"
using namespace std;

class GPDict : public RuleGenerator
{
public:
	GPDict();
	vector<t_rule> gen_rules();

private:
	vector<t_rule> rules;
};


#endif //XCLEAN_GPDICT_H
