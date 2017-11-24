//
// Created by Wenbo Tao on 6/9/17.
//

#ifndef XCLEAN_HANDDICT_H
#define XCLEAN_HANDDICT_H


#include "RuleGenerator.h"
using namespace std;

class HandDict : public RuleGenerator
{
public:
	HandDict() {}
	HandDict(string);
	vector<t_rule> gen_rules();

private:
	vector<t_rule> rules;
};


#endif //XCLEAN_HANDDICT_H
