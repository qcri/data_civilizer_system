//
// Created by Wenbo Tao on 2/11/17.
//

#ifndef XCLEAN_VLDB09_H
#define XCLEAN_VLDB09_H
#include "RuleGenerator.h"
using namespace std;


class Vldb09 : public RuleGenerator
{
public:
	//constructor
	Vldb09(vector<string>);

	//overwrite virtual method
	vector<t_rule> gen_rules();
};


#endif //XCLEAN_VLDB09_H
